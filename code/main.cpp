#define NOMINMAX
#include <windows.h>
#include <windowsx.h>

#include <stdio.h>
#include <GL/GL.h>

#include "misc.h"
#include "logging.h"
#include "memory.h"
//#include "maths.h"
#include "gl_extension_loading.h"
#include "gl_graphics.h"

__inline uint get_cd(char* output)
{
    return GetCurrentDirectory(GetCurrentDirectory(0,0), output);
}

__inline void set_cd(char* directory_name)
{
    SetCurrentDirectory(directory_name);
}

__inline int load_file(char* filename, char* output)
{
    HANDLE file = CreateFile(filename,
                             GENERIC_READ,
                             FILE_SHARE_READ, 0,
                             OPEN_EXISTING,
                             FILE_FLAG_SEQUENTIAL_SCAN, 0);

    if(file == INVALID_HANDLE_VALUE)
    {
        log_error("windows code ", GetLastError(), ", file ", filename, " could not be found\n");
        exit(EXIT_SUCCESS);
    }

    LARGE_INTEGER file_size;
    auto error = GetFileSizeEx(file, &file_size);

    if(!error)
    {
        log_error(GetLastError(), " opening file\n");
        exit(EXIT_SUCCESS);
    }

    DWORD bytes_read;
    error = ReadFile(file,
                     output,
                     file_size.LowPart,
                     &bytes_read,
                     0);
    if(!error)
    {
        log_error("error reading file\n");
        exit(EXIT_SUCCESS);
    }
    CloseHandle(file);

    return bytes_read;
}

#include "cl_compute_layer.h"

uint window_width = w;
uint window_height = h;

WINDOWPLACEMENT g_wpPrev = { sizeof(g_wpPrev) };

void fullscreen(HWND hwnd)
{
    DWORD dwStyle = GetWindowLong(hwnd, GWL_STYLE);
    if (dwStyle & WS_OVERLAPPEDWINDOW) {
        MONITORINFO mi = { sizeof(mi) };
        if (GetWindowPlacement(hwnd, &g_wpPrev) &&
            GetMonitorInfo(MonitorFromWindow(hwnd,
                                             MONITOR_DEFAULTTOPRIMARY), &mi)) {
            SetWindowLong(hwnd, GWL_STYLE,
                          dwStyle & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(hwnd, HWND_TOP,
                         mi.rcMonitor.left, mi.rcMonitor.top,
                         mi.rcMonitor.right - mi.rcMonitor.left,
                         mi.rcMonitor.bottom - mi.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    } else {
        SetWindowLong(hwnd, GWL_STYLE,
                      dwStyle | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(hwnd, &g_wpPrev);
        SetWindowPos(hwnd, NULL, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        case WM_SIZE:
            window_width = LOWORD(lParam);
            window_height = HIWORD(lParam);
            glViewport(0, 0, LOWORD(lParam),HIWORD(lParam));
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)
{
    initialize_compute();

    WNDCLASSEX wc;
    {//init the window class
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.style = CS_OWNDC;
        wc.lpfnWndProc = WndProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = hInstance;
        wc.hIcon = LoadIcon(0, IDI_APPLICATION);
        wc.hCursor = LoadCursor(0, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
        wc.lpszMenuName = 0;
        wc.lpszClassName = "class name";
        wc.hIconSm = LoadIcon(0, IDI_APPLICATION);

        if(!RegisterClassEx(&wc))
        {
            MessageBox(0, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
            return 0;
        }
    }

    HWND hwnd;
    {//init hwnd
        hwnd = CreateWindowEx(
            WS_EX_APPWINDOW, //extended window style
            "class name", //the class name
            "Field Simulation", //The window title
            WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, //window style
            CW_USEDEFAULT, CW_USEDEFAULT, //position
            window_width, window_height, //size
            0, //handle to the parent window, this has no parents
            0, //menu handle
            hInstance, //duh
            0 //lparam
            );
        if(hwnd == 0)
        {
            MessageBox(0, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
            return 0;
        }
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    {//GL context
        HDC dummy_dc;
        {
            dummy_dc = GetDC(hwnd);

            PIXELFORMATDESCRIPTOR pfd =
                {
                    sizeof(PIXELFORMATDESCRIPTOR),
                    1,
                    PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    //Flags
                    PFD_TYPE_RGBA,            //The kind of framebuffer. RGBA or palette.
                    32,                        //Colordepth of the framebuffer.
                    0, 0, 0, 0, 0, 0,
                    0,
                    0,
                    0,
                    0, 0, 0, 0,
                    32,                        //Number of bits for the depthbuffer
                    0,                        //Number of bits for the stencilbuffer
                    0,                        //Number of Aux buffers in the framebuffer.
                    PFD_MAIN_PLANE,
                    0,
                    0, 0, 0
                };

            const int pf = ChoosePixelFormat(dummy_dc, &pfd);
            SetPixelFormat(dummy_dc, pf, &pfd);
        }
        HGLRC glrc = wglCreateContext(dummy_dc);

        wglMakeCurrent(dummy_dc, glrc);

        loadGLFunctions();

        printf("%s, %s\n", (char*)glGetString(GL_VERSION), "OPENGL VERSION");

        wglMakeCurrent(0, 0);
        wglDeleteContext(glrc);
    }

    HDC dc;
    {
        dc = GetDC(hwnd);

        const int attribList[] = {
            WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
            WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
            WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
            WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
            WGL_COLOR_BITS_ARB, 32,
            WGL_DEPTH_BITS_ARB, 32,
            WGL_STENCIL_BITS_ARB, 0,
            0,        //End
        };

        int pixFormat;
        UINT numFormats;

        wglChoosePixelFormatARB(dc, attribList, 0, 1, &pixFormat, &numFormats);

        PIXELFORMATDESCRIPTOR pfd;
        SetPixelFormat(dc, pixFormat, &pfd);

        int attribs[] = {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
            WGL_CONTEXT_MINOR_VERSION_ARB, 4,
            WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
            WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
            0
        };

        HGLRC glrc = wglCreateContextAttribsARB(dc, 0, attribs);

        wglMakeCurrent(dc, glrc);
    }

    #ifdef DEBUG
    glDebugMessageCallbackARB(gl_error_callback, 0);
    glEnable(GL_DEBUG_OUTPUT);
    #endif

    memory_size = 1*gigabyte;
    start_memory = VirtualAlloc(0, memory_size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    free_memory = start_memory;
    assert(free_memory);

    vi_buffer * vi_buffers;
    size_t n_vi_buffers;
    {
        all_vi_buffers all_buffers = init_vertex_and_index_buffers(stalloc(100*megabyte));
        vi_buffers = all_buffers.buffers;
        n_vi_buffers = all_buffers.n;
    }

    GLuint program;
    GLuint transform_uniform;
    {
        char* vertex_shader_source = (char*) free_memory;
        size_t vertex_shader_source_size = load_file("../code/vertex_shader.glsl", vertex_shader_source);
        vertex_shader_source[vertex_shader_source_size] = 0;
        free_memory = (void*)((char*)free_memory+vertex_shader_source_size+1);

        char* fragment_shader_source = (char*) free_memory;
        size_t fragment_shader_source_size = load_file("../code/fragment_shader.glsl", fragment_shader_source);
        fragment_shader_source[fragment_shader_source_size] = 0;
        free_memory = (void*)((char*)free_memory+fragment_shader_source_size+1);

        log_output("vertex shader:\n", vertex_shader_source, "\nfragmend shader:\n", fragment_shader_source);

        program = init_program(vertex_shader_source, fragment_shader_source);

        transform_uniform = glGetUniformLocation(program, "t");
    }

    GLuint fbo;
    GLuint colorbuffer;
    GLuint depthbuffer;
    {//TODO: use direct state access?
        uint n_samples = 8;
        printf("%dX Multisampling\n", n_samples);

        //fbo
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        //color buffer
        glGenRenderbuffers(1, &colorbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, colorbuffer);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, n_samples, GL_RGBA8, window_width, window_height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorbuffer);

        // //depth buffer
        glGenRenderbuffers(1, &depthbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, depthbuffer);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, n_samples, GL_DEPTH_COMPONENT32, window_width, window_height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthbuffer);

        //errors
        int error = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        assert(error == GL_FRAMEBUFFER_COMPLETE);
    }

    {
        // glEnable(GL_DEPTH_TEST);
        // glDepthMask(GL_TRUE);
        // glDepthFunc(GL_LEQUAL);
        // glDepthRange(0.0f, 1.0f);
        // glEnable(GL_DEPTH_CLAMP);

        // glFrontFace(GL_CCW);
        // glCullFace(GL_BACK);
        // //glEnable(GL_CULL_FACE);

        glLineWidth(1.0);

        glClearColor(0.0/255.0, 50.0/255.0, 98.0/255.0, 1.0);
        glClearDepth(1.0);

        wglSwapIntervalEXT(0);
    }

    LARGE_INTEGER timer_frequency;
    LARGE_INTEGER last_time = {0};
    LARGE_INTEGER this_time = {0};

    QueryPerformanceFrequency(&timer_frequency);

    MSG msg;
    do
    {//main loop
        while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);

            switch(msg.message)
            {
                case WM_DESTROY:
                {
                    PostQuitMessage(0);
                    break;
                }
                case WM_QUIT:
                    return 0;
                default:
                    DispatchMessage(&msg);
            }
        }

        simulate();

        #if 1
        int* bitmap = (int*) malloc(sizeof(int)*w*h);
        for(int i = 0; i < h*w; i++)
        {
            // float scale = 100.0;
            float psir = data[i];
            float psii = (data+dim)[i];
            #if 1
            float r = clamp(psir,0,1);//clamp(scale*psir+0.5, 0.0, 1.0);
            float b = clamp(-psir,0,1);//clamp(scale*psii+0.5, 0.0, 1.0);
            float x = i%w;
            float y = i/w;
            float g = 0.0;//clamp(3*(sq(x-w/2)+sq(y-h/2))/(w*h), 0.0, 1.0);
            // r = pow(r, 1.0/2.2);
            // b = pow(b, 1.0/2.2);
            // g = pow(g, 1.0/2.2);
            #else
            float P = sq(psir)+sq(psii);
            float r = clamp(scale*P, 0.0, 1.0);
            float b = clamp(scale*P, 0.0, 1.0);
            float g = 0.0;
            #endif
            bitmap[i] = ((0xFF&(int)(0xFF*b))
                         |(0xFF&(int)(0xFF*g))<<(8)
                         |(0xFF&(int)(0xFF*r))<<(8*2));
        }

        BITMAPINFO bmi;
        bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
        bmi.bmiHeader.biWidth = w;
        bmi.bmiHeader.biHeight = -h;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;

        RECT window_rect;
        GetClientRect(hwnd, &window_rect);

        StretchDIBits(dc,
                      (window_rect.right-window_rect.left-w)/2, (window_rect.bottom-window_rect.top-h)/2,
                      w, h,
                      0, 0, w, h,
                      bitmap,
                      &bmi,
                      DIB_RGB_COLORS,
                      SRCCOPY);
        #endif
        // glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        // glViewport(0, 0, window_width, window_height);
        // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // glUseProgram(program);

        // float camera[16] = {};
        // for(int i = 0; i < 16; i+=5)
        // {
        //     camera[i] = 1.0;
        // }

        // glUniformMatrix4fv(transform_uniform, 1, false, (float *) &camera);
        // bind_vertex_and_index_buffers(vi_buffers[vi_id_cube].vb, vi_buffers[vi_id_cube].ib);
        // glDrawElements(GL_TRIANGLES, vi_buffers[vi_id_cube].n, GL_UNSIGNED_SHORT, 0);

        // //render fbo
        // #if 1
        // glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        // glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
        // glDrawBuffer(GL_BACK);                       // Set the back buffer as the draw buffer

        // glBlitFramebuffer(0, 0, window_width, window_height,
        //                   0, 0, window_width, window_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        // #else
        // glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // glViewport(0, 0, window_width, window_height);
        // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // glUseProgram(post_program);

        // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // glActiveTexture(GL_TEXTURE0);
        // glBindTexture(GL_TEXTURE_2D, colorbuffer);
        // glUniform1i(post_tex_uniform, 0);

        // glUniform1i(width_uniform, window_width);
        // glUniform1i(height_uniform, window_height);

        // glBegin(GL_TRIANGLE_FAN);

        // glVertex2f( 1.0,  1.0);
        // glVertex2f(-1.0,  1.0);
        // glVertex2f(-1.0, -1.0);
        // glVertex2f( 1.0, -1.0);

        // glEnd();
        // #endif
        // SwapBuffers(dc);
    }
    while(msg.message != WM_QUIT);

    return 0;
}
