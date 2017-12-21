#ifndef CL_EXTENSION_LOADING
#define CL_EXTENSION_LOADING

#include <CL/CL.h>
#define clGetGLContextInfoKHR clGetGLContextInfoKHR_proc
#include <CL/cl_gl.h>
#undef clGetGLContextInfoKHR

#include "misc.h"

/* #define cl_load_operation(rval, ext, args)  typedef rval (apientry * ext##_func)args; */
/* #include "cl_functions_list.h" */

clGetGLContextInfoKHR_fn clGetGLContextInfoKHR = 0;

void cl_load_functions()
{
    #define cl_load_operation(ret, func, args) func = (CONCAT(func, _fn)) clGetExtensionFunctionAddress(STR(func)); assert(func, STR(func) " could not be loaded");
    #include "cl_functions_list.h"
}

#endif //CL_EXTENSION_LOADING
