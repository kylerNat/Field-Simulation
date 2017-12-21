#include <CL/cl.h>
#include "cl_extension_loading.h"
#include "maths.h"

#include <time.h>

int w = 500;
int h = 500;

real* data;

/* real error_function() */
/* { */
/*     return hbar^2/(2.0*m)*laplacian(psi) + (V-E)*psi; */
/* } */


local cl_kernel kernel;
local uint data_size;
local cl_mem psi[2];
local cl_command_queue queue;
void init_compute(GLenum texture_target, GLuint* texture, HGLRC glrc, HDC dc)
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

    cl_device_id* devices = (cl_device_id*) stalloc(devices_size);
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
    cl_context context = clCreateContext(context_properties,
                                         1, //n_devices,
                                         &devices[0],
                                         0, //notify callback
                                         0, //used data for notify callback
                                         &error);
    assert(error==0, "Could not create context");

    //create device queue
    queue = clCreateCommandQueue(context, devices[0], 0, &error);
    if(error < 0) log_error("Could not create queue");

    //compile kernel
    char* code = (char*) free_memory;
    size_t code_size = load_file("../code/spring_bed_kernel.cl", code);

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

    kernel = clCreateKernel(program, "simulate", &error);
    if(error < 0) log_error("Could not create kernel");

    psi[0] = clCreateFromGLTexture(context,
                                 CL_MEM_READ_WRITE,
                                 texture_target,
                                 0,
                                 texture[0],
                                 &error);
    assert(error==CL_SUCCESS, error, ", Could not create buffer for psi[0]");

    psi[1] = clCreateFromGLTexture(context,
                                 CL_MEM_READ_WRITE,
                                 texture_target,
                                 0,
                                 texture[1],
                                 &error);
    assert(error==CL_SUCCESS, error, ", Could not create buffer for psi[1]");
}

void simulate()
{
    static int i = 0;
    real dt = 0.001;
    int error;

    glFinish();

    error = clEnqueueAcquireGLObjects(queue, 2,  psi, 0, 0, 0);
    assert(error==CL_SUCCESS, error, ", Could not aquire gl objects");

    #define clSetKernelArgAndAssert(kernel, arg, size, value)           \
        error = clSetKernelArg(kernel, arg, size, value);               \
        assert(error==CL_SUCCESS, error, ", Could not set kernal arg " STR(arg) " to " STR(value));

    for(int j = 0; j < 20; j++)
    {
        size_t global_size[] = {w, h};
        size_t local_size[] = {0, 0};

        clSetKernelArgAndAssert(kernel, 0, sizeof(real), &dt);
        clSetKernelArgAndAssert(kernel, 1, sizeof(cl_mem), &psi[j%2]);
        clSetKernelArgAndAssert(kernel, 2, sizeof(cl_mem), &psi[1-j%2]);

        error = clEnqueueNDRangeKernel(queue, kernel, 2, 0, global_size, 0, 0, 0, 0);
        if(error < 0) log_error("Could not enqueue the kernel");
        i++;
    }

    clFinish(queue);
    clEnqueueReleaseGLObjects(queue, 2, psi, 0, 0, 0);
}
