#include <CL/cl.h>
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
local cl_mem psi_r[2];
local cl_mem psi_i[2];
local cl_command_queue queue;
void initialize_compute()
{
    log_output("initialize_compute running...\n");
    //find device
    cl_platform_id platforms[1]; //TODO: actually look through platform list
    uint n_platforms = 0;

    cl_int platform_ids = clGetPlatformIDs(len(platforms),
                                           platforms,
                                           &n_platforms);

    cl_platform_id platform = platforms[0];

    cl_device_id device_list[12];
    uint n_device_list = 0;

    cl_int device_ids = clGetDeviceIDs(platform,
                                       len(device_list),
                                       CL_DEVICE_TYPE_ALL,
                                       device_list,
                                       &n_device_list);

    cl_device_id devices[] = {device_list[0]}; //Devices to actually use
    uint n_devices = len(devices);

    //create context
    cl_context_properties context_properties[] = {
        CL_CONTEXT_PLATFORM, (cl_context_properties) platform,
        0,
    };

    int error;
    cl_context context = clCreateContext(context_properties,
                                         1,//n_devices,
                                         &devices[0],
                                         0, //notify callback
                                         0, //used data for notify callback
                                         &error);
    if(error < 0) log_error("Could not create context");

    void* free_mem = malloc(1024*1024);//TODO: fix memory stuff
    char* code = (char*) free_mem;
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

    #define pi (3.14159265358979323846264338327950)

    #define dim (w*h)
    data_size = sizeof(real)*dim;
    data = (real*) calloc(4*dim, sizeof(real));
    srand(time(0));
    /* for(real theta = 0; theta < 2*pi; theta += pi/1000.0) */
    /* { */
    /*     real r = 50+10*(rand()%1000-500)/1000.0; */
    /*     int dx = r*cos(theta); */
    /*     int dy = r*sin(theta); */

    /*     int x = dx + w/2; */
    /*     int y = dy + h/2; */

    /*     if(x >= 0 && x < w && y >= 0 && y < h) data[x+y*w] = 10.0; */
    /* } */
    for(int x = 0; x < w; x++)
        for(int y = 0; y < h; y++)
        {
            int dx = x-w/2;
            int dy = y-h/2;

            data[x+y*w] = sin((2*pi*x)/w)*cos((pi*(2*y+x))/h);
            /* data[x+y*w] = (100.0*dx*dy)/(w*h); */
            /* data[x+y*w] = 10*(sq((float)(dx*dx+dy*dy)-0.1*(w*h)) < 100000 &&x!=0&&x!=w-1); */
            /* data[x+y*w] = (x!=0&&x!=w-1)*0.1*(rand()%1000-500); */
            /* data[x+y*w] = sin((2*pi*x)/w)*cos((2*pi*y)/h); */

            /* data[(w/2+dx)+(w/2+dy)*w] = 10.5*exp(-0.1*(sq(dx)+sq(dy))); */

            /* data[(x)+(y)*w]       = 0.5*exp(-0.01*(sq(dx)+sq(dy)))*cos(1.0*dy); */
            /* (data+dim)[(x)+(y)*w] = 0.5*exp(-0.01*(sq(dx)+sq(dy)))*sin(1.0*dy); */

            if(x==0||x==w-1) data[x+y*w] = 0;
        }

    psi_r[0] = clCreateBuffer(context,
                              CL_MEM_COPY_HOST_PTR,
                              data_size,
                              data,
                              &error);
    if(error < 0) log_error("Could not create buffer for psi_r[0]");

    psi_r[1] = clCreateBuffer(context,
                              CL_MEM_COPY_HOST_PTR,
                              data_size,
                              data+2*dim,
                              &error);
    if(error < 0) log_error("Could not create buffer for psi_r[1]");

    psi_i[0] = clCreateBuffer(context,
                              CL_MEM_COPY_HOST_PTR,
                              data_size,
                              data+1*dim,
                              &error);
    if(error < 0) log_error("Could not create buffer for psi_i[0]");

    psi_i[1] = clCreateBuffer(context,
                            CL_MEM_COPY_HOST_PTR,
                            data_size,
                            data+3*dim,
                            &error);
    if(error < 0) log_error("Could not create buffer for psi_i[1]");

    queue = clCreateCommandQueue(context, devices[0], 0, &error);
    if(error < 0) log_error("Could not create queue");
}

void simulate()
{
    static int i = 0;
    real dt = 0.001;
    int error;

    for(int j = 0; j < 20; j++)
    {
        error  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &psi_r[i&1]);
        error |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &psi_i[i&1]);
        error |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &psi_r[!(i&1)]);
        error |= clSetKernelArg(kernel, 3, sizeof(cl_mem), &psi_i[!(i&1)]);
        error |= clSetKernelArg(kernel, 4, sizeof(real), &dt);
        if(error < 0) log_error("Could not set kernal args");
        size_t global_size[] = {w, h};
        size_t local_size[] = {0, 0};
        error = clEnqueueNDRangeKernel(queue, kernel, 2, 0, global_size, 0, 0, 0, 0);
        if(error < 0) log_error("Could not enqueue the kernel");
        i++;
    }

    error = clEnqueueReadBuffer(queue, psi_r[0], CL_TRUE, 0, data_size, data, 0, 0, 0);
    if(error < 0) log_error("Could not read buffer");
    error = clEnqueueReadBuffer(queue, psi_i[0], CL_TRUE, 0, data_size, data+dim, 0, 0, 0);
    if(error < 0) log_error("Could not read buffer");
}
