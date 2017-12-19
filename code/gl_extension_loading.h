#ifndef GL_EXTENSION_LOADING
#define GL_EXTENSION_LOADING

#include <GL/GL.h>
#include "gl/glext.h"
#include "gl/wglext.h"

#include "misc.h"

#define gl_load_operation(rval, ext, args)  typedef rval (APIENTRY * ext##_Func)args; ext##_Func ext = 0;
#include "gl_functions_list.h"
#endif

void loadGLFunctions()
{
    HMODULE gl_module_handle = GetModuleHandle("opengl32");
    
    // this is pointer to function which returns pointer to string with list of all wgl extensions
    PFNWGLGETEXTENSIONSSTRINGEXTPROC wglGetExtensionsStringEXT = NULL;
    wglGetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC) wglGetProcAddress("wglGetExtensionsStringEXT");
    
    const char* extensions_list = wglGetExtensionsStringEXT();
    
    #define gl_check(extension) if(strstr(extensions_list, extension))
    #define gl_load_operation(ret, func, args) func = (CONCAT(func, _Func)) wglGetProcAddress(STR(func)); assert(func, STR(func) "could not be loaded");
    #include "gl_functions_list.h"
}
