#include <CL/cl.h>
#include "cl_extension_loading.h"
#include "maths.h"

#include <time.h>

int w = 500;
int h = 500;

#define n_textures 2

//TODO: abstractify

cl_command_queue queue;
cl_context context;
cl_device_id* devices;
void init_compute(HGLRC glrc, HDC dc)
{
    cl_load_functions();

    int error;
    log_output("initialize_compute running...\n");

    //TODO: make sure extensions are supported by device
    //find device
    cl_platform_id platforms[10]; //TODO: actually look through platform list
    uint n_platforms = 0;

    cl_int platform_ids = clGetPlatformIDs(len(platforms),
                                           platforms,
                                           &n_platforms);

    log_output("found ", n_platforms, " platforms\n");
    for(int i = 0; i < n_platforms; i++)
    {
        size_t platform_name_size;
        clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, 0, 0, &platform_name_size);
        char* platform_name = (char*) free_memory;
        clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, platform_name_size, platform_name, 0);
        log_output("    platform ", i, ": \n        name: ", platform_name, "\n");
    }

    cl_platform_id platform = platforms[0];

    cl_context_properties context_properties[] = {
        CL_CONTEXT_PLATFORM, (cl_context_properties) platform,
        CL_GL_CONTEXT_KHR, (cl_context_properties) glrc,
        CL_WGL_HDC_KHR, (cl_context_properties) dc,
        0,
    };

    //TODO: actually look through device list
    size_t devices_size = 0;
    error = clGetGLContextInfoKHR(context_properties,
                                  CL_CURRENT_DEVICE_FOR_GL_CONTEXT_KHR,
                                  0,
                                  0,
                                  &devices_size);
    assert(error==CL_SUCCESS, error, ", Could not get cl device for current gl context");

    devices = (cl_device_id*) stalloc(devices_size);
    clGetGLContextInfoKHR(context_properties,
                          CL_CURRENT_DEVICE_FOR_GL_CONTEXT_KHR,
                          devices_size,
                          devices,
                          0);

    size_t n_devices = devices_size/sizeof(cl_device_id);

    log_output("found ", n_devices, " devices for current gl context\n");
    log_output("devices:\n");
    for(int i = 0; i < n_devices; i++)
    {
        cl_device_type device_type;
        clGetDeviceInfo(devices[i], CL_DEVICE_TYPE, sizeof(cl_device_type), &device_type, 0);
        size_t extensions_size;
        clGetDeviceInfo(devices[i], CL_DEVICE_EXTENSIONS, 0, 0, &extensions_size);
        char* extensions = (char*) free_memory;
        clGetDeviceInfo(devices[i], CL_DEVICE_EXTENSIONS, extensions_size, extensions, 0);
        log_output("    device ", (int)i, ":\n        type: ", (int)device_type, "\n        extensions: ", extensions, "\n");
    }

    //create context
    context = clCreateContext(context_properties,
                                         1, //n_devices,
                                         &devices[0],
                                         0, //notify callback
                                         0, //used data for notify callback
                                         &error);
    assert(error==0, "Could not create context");

    //create device queue
    queue = clCreateCommandQueue(context, devices[0], 0, &error);
    if(error < 0) log_error("Could not create queue");
}

cl_kernel compile_kernel(char* code, size_t code_size)
{
    int error;

    //TODO: actually check for errors
    cl_program program = clCreateProgramWithSource(context,
                                                   1,
                                                   (const char**) &code,
                                                   &code_size,
                                                   &error);
    if(error < 0) log_error("Could not create program");

    error = clBuildProgram(program, 0, 0, 0, 0, 0);
    if(error < 0) {
        size_t log_size;
        clGetProgramBuildInfo(program, devices[0], CL_PROGRAM_BUILD_LOG,
                              0, NULL, &log_size);
        char* program_log = (char*) malloc(log_size + 1);
        program_log[log_size] = '\0';
        clGetProgramBuildInfo(program, devices[0], CL_PROGRAM_BUILD_LOG,
                              log_size + 1, program_log, NULL);
        log_error("\n", program_log, "\n");
    }

    cl_kernel kernel = clCreateKernel(program, "simulate", &error);
    if(error < 0) log_error("Could not create kernel");

    return kernel;
}

cl_mem psi[n_textures];
void cl_buffers_from_gl_textures(GLenum texture_target, GLuint* texture)
{
    int error;

    for(int i = 0; i < n_textures; i++)
    {
        psi[i] = clCreateFromGLTexture(context,
                                     CL_MEM_READ_WRITE,
                                     texture_target,
                                     0,
                                     texture[i],
                                     &error);
        assert(error==CL_SUCCESS, error, ", Could not create buffer for psi[", i ,"]");
    }
}
