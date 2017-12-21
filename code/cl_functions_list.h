#ifdef cl_khr_gl_sharing
    cl_load_operation(cl_int, clGetGLContextInfoKHR, (const cl_context_properties * properties, cl_gl_context_info param_name, size_t param_value_size, void * param_value, size_t * param_value_size_ret));
#else
#error cl_khr_gl_sharing is not availible
#endif

#undef cl_load_operation
