#define NOMINMAX
#include <windows.h>
#include <windowsx.h>

#include <stdio.h>
#include <GL/GL.h>

#include "misc.h"
#include "logging.h"
#include "memory.h"
#include "maths.h"
#include "gl_extension_loading.h"

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

char* load_file_0_terminated(char* filename)
{
    char* output = (char*) free_memory;
    size_t output_size = load_file(filename, output);
    output[output_size] = 0;
    free_memory = (void*)((char*)free_memory+output_size+1);
    return output;
}

#include "gl_graphics.h"
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
        // case WM_CLOSE:
        // {
        //     DestroyWindow(hwnd);
        //     break;
        // }
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            break;
        }
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

    HGLRC glrc;
    HDC dc;
    {//init dc & gl context
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

        glrc = wglCreateContextAttribsARB(dc, 0, attribs);

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

    real* data;
    GLuint data_texture[n_textures];
    cl_kernel kernel;
    {//initialize compute stuff
        #define stride 4/n_textures
        #define dim (w*h)
        size_t data_size = sizeof(real)*dim;
        data = (real*) calloc(4*dim, sizeof(real)); //TODO: fix memory management
        srand(time(0));

        //set up initial conditions
        for(int x = 0; x < w; x++)
            for(int y = 0; y < h; y++)
            {
                int dx = x-w/2;
                int dy = y-h/2;

                // data[(x+y*w)*stride] = 10*exp(-0.001*(sq(dx)+sq(dy)));
                // data[(x+y*w)*stride] = sin((2*pi*x)/w)*cos((pi*(2*y+x))/h);
                // data[(x+y*w)*stride] = (1.0*dx*dy)/(w*h);
                // data[(x+y*w)*stride] += (sq((real)(dx*dx+dy*dy)-0.1*(w*h)) < 100000 &&x!=0&&x!=w-1);
                // data[(x+y*w)*stride] += (x!=0&&x!=w-1)*0.0001*(rand()%1000-500);
                // data[(x+y*w)*stride] = sin((2*pi*x)/w)*cos((2*pi*y)/h);

                // data[(w/2+dx)+(w/2+dy)*w] = 10.5*exp(-0.1*(sq(dx)+sq(dy)));

                /* data[(x)+(y)*w]       = 0.5*exp(-0.01*(sq(dx)+sq(dy)))*cos(1.0*dy); */
                /* (data+dim)[(x)+(y)*w] = 0.5*exp(-0.01*(sq(dx)+sq(dy)))*sin(1.0*dy); */
            }

        for(int i = 0; i < n_textures; i++)
        {
            glGenTextures(1, &data_texture[i]);
            glBindTexture(GL_TEXTURE_2D, data_texture[i]);

            //texture parameters
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            //allocate and upload
            // uint n_layers = 1;
            // glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA32F, width, height, n_layers);
            // glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, w, h, n_layers, GL_RGBA, GL_FLOAT, data);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, w, h, 0, GL_RG, GL_FLOAT, data+i*(w*h*stride));
        }

        init_compute(glrc, dc);

        //load kernel source
        char* code = (char*) free_memory;
        size_t code_size = load_file("../code/spring_bed_kernel.cl", code);

        kernel = compile_kernel(code, code_size);
        cl_buffers_from_gl_textures(GL_TEXTURE_2D, data_texture);
    }

    vi_buffer * vi_buffers;
    size_t n_vi_buffers;
    {
        all_vi_buffers all_buffers = init_vertex_and_index_buffers(stalloc(100*megabyte));
        vi_buffers = all_buffers.buffers;
        n_vi_buffers = all_buffers.n;
    }

    GLuint program;
    GLuint transform_uniform;
    GLuint data_uniform;
    {
        shader_source sources[] = {
            load_shader_from_file(GL_VERTEX_SHADER, "../code/vertex_shader.glsl"),
            load_shader_from_file(GL_TESS_CONTROL_SHADER, "../code/tessellation_control_shader.glsl"),
            load_shader_from_file(GL_TESS_EVALUATION_SHADER, "../code/tessellation_evaluation_shader.glsl"),
            load_shader_from_file(GL_FRAGMENT_SHADER, "../code/fragment_shader.glsl"),
        };

        program = init_program(sources, len(sources));

        transform_uniform = glGetUniformLocation(program, "t");
        data_uniform = glGetUniformLocation(program, "data");
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
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LEQUAL);
        glDepthRange(0.0f, 1.0f);
        glEnable(GL_DEPTH_CLAMP);

        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glFrontFace(GL_CCW);
        // glCullFace(GL_BACK);
        // glEnable(GL_CULL_FACE);

        glLineWidth(1.0);

        glEnable(GL_FRAMEBUFFER_SRGB);
        #define gamma 2.2
        glClearColor(pow(0.2, gamma), pow(0.0, gamma), pow(0.3, gamma), 1.0);
        glClearDepth(1.0);

        wglSwapIntervalEXT(1);
    }

    LARGE_INTEGER timer_frequency;
    LARGE_INTEGER last_time = {0};
    LARGE_INTEGER this_time = {0};

    QueryPerformanceFrequency(&timer_frequency);

    // enum button_code
    // {
    //     button_lmb,
    //     button_rmb,
    //     button_count,
    // };
    // byte buttons[(button_count-1)/8+1];
    // #define set_button_off(code) (buttons[code/8] |= 1<<code%8)
    // #define set_button_on(code) (buttons[code/8] &= ~(1<<code%8))
    // #define get_button(code) ((buttons[code/8]>>(code%8))&1)

    MSG msg;
    do
    {//main loop
        while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);

            switch(msg.message)
            {
                // #define process_mouse_button(case_prefix, code) \
                //     case case_prefix##DOWN:                     \
                //     {                                           \
                //         set_button_on(code);                    \
                //         break;                                  \
                //     }                                           \
                // case case_prefix##UP:                           \
                // {                                               \
                //     set_button_off(code);                       \
                //     break;                                      \
                // }

                // process_mouse_button(WM_LBUTTON, button_lmb);
                // process_mouse_button(WM_RBUTTON, button_rmb);

                case WM_KEYDOWN:
                {
                    break;
                }
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

        {//simulate
            static int i = 0;
            real dt = 0.001;
            int error;

            glFinish();

            error = clEnqueueAcquireGLObjects(queue, n_textures,  psi, 0, 0, 0);
            assert(error==CL_SUCCESS, error, ", Could not aquire gl objects");

            #define clSetKernelArgAndAssert(kernel, arg, size, value)   \
                error = clSetKernelArg(kernel, arg, size, value);       \
                assert(error==CL_SUCCESS, error, ", Could not set kernal arg " STR(arg) " to " STR(value));

            for(int j = 0; j < 200; j++)
            {
                size_t global_size[] = {w, h};
                size_t local_size[] = {0, 0};

                clSetKernelArgAndAssert(kernel, 0, sizeof(real), &dt);
                clSetKernelArgAndAssert(kernel, 1, sizeof(cl_mem), &psi[j%2]);
                clSetKernelArgAndAssert(kernel, 2, sizeof(cl_mem), &psi[1-j%2]);

                /* clSetKernelArgAndAssert(kernel, 0, sizeof(real), &dt); */
                /* clSetKernelArgAndAssert(kernel, 1, sizeof(cl_mem), &psi[j%2]); */
                /* clSetKernelArgAndAssert(kernel, 2, sizeof(cl_mem), &psi[2+j%2]); */
                /* clSetKernelArgAndAssert(kernel, 3, sizeof(cl_mem), &psi[1-j%2]); */
                /* clSetKernelArgAndAssert(kernel, 4, sizeof(cl_mem), &psi[2+1-j%2]); */

                error = clEnqueueNDRangeKernel(queue, kernel, 2, 0, global_size, 0, 0, 0, 0);
                if(error < 0) log_error("Could not enqueue the kernel");
                i++;
            }

            clFinish(queue);
            clEnqueueReleaseGLObjects(queue, n_textures, psi, 0, 0, 0);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glViewport(0, 0, window_width, window_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(program);

        #define button_rotate VK_LBUTTON
        #define button_poke VK_RBUTTON

        POINT cursor_point;
        RECT window_rect;
        float dx_m;
        float dy_m;
        {
            static POINT old_cursor_point;
            GetCursorPos(&cursor_point);
            dx_m = cursor_point.x-old_cursor_point.x;
            dy_m = cursor_point.y-old_cursor_point.y;
            GetWindowRect(hwnd, &window_rect);
            old_cursor_point = cursor_point;
        }

        static real phi = 0.0;
        static real theta = 0.0;

        HWND foreground_window = GetForegroundWindow();
        if(GetAsyncKeyState(button_rotate) && foreground_window == hwnd)
        {
            phi += 0.01*dx_m;
            theta -= 0.01*dy_m;
        }

        // phi += 0.01;
        //NOTE: the matrices are stored in column major order
        real4x4 R_z = {
            cos(phi),+sin(phi), 0.0, 0.0,
           -sin(phi), cos(phi), 0.0, 0.0,
            0.0   ,    0.0, 1.0, 0.0,
            0.0   ,    0.0, 0.0, 1.0,
        };
        real4x4 R_x = {
            1.0,    0.0,    0.0, 0.0,
            0.0, cos(theta), sin(theta), 0.0,
            0.0,-sin(theta), cos(theta), 0.0,
            0.0,    0.0,    0.0, 1.0,
        };
        real4x4 camera = R_x*R_z;

        static bool poking = false;
        if(GetAsyncKeyState(button_poke) && foreground_window == hwnd)
        {
            if(!poking)
            {
                real4 r_m = {cursor_point.x-(window_rect.left+window_rect.right)/2,
                             -cursor_point.y+(window_rect.bottom+window_rect.top)/2,
                             0.0, 0.0};
                r_m.z = +r_m.y/cos(theta); //TODO: generalize this, I need to project the mouse position onto a plane
                // for(int i = 0; i < 4; i++)
                // {
                //     for(int j = 0; j < 4; j++)
                //         log_output((transpose(camera))[i][j], ", ");
                //     log_output("\n");
                // }
                real4 r_t = transpose(camera)*r_m; //assumes camera is a unitary matrix
                real x = r_t.x+w/2;
                real y = r_t.y+h/2;

                glGetTextureSubImage(data_texture[0], 0, 0, 0, 0, w, h, 1, GL_RG, GL_FLOAT, w*h*2*sizeof(float), data);
                for(int tx = 0; tx < w; tx++)
                    for(int ty = 0; ty < h; ty++)
                    {
                        data[(tx+ty*h)*2] -= 1*exp(-(sq(tx-x)+sq(ty-y))/1000.0);
                        // data[(tx+ty*h)*2] -= 1*exp(-(sq(tx-x)+sq(ty-y))/10000.0)*cos(float(tx-x)*pi/(10.0));
                        // data[(tx+ty*h)*2+1] -= 1*exp(-(sq(tx-x)+sq(ty-y))/10000.0)*sin(float(tx-x)*pi/(10.0));
                    }
                glTextureSubImage2D(data_texture[0], 0, 0, 0, w, h, GL_RG, GL_FLOAT, data);
            }
            poking = true;
        }
        else poking = false;

        glBindTexture(GL_TEXTURE_2D, data_texture[0]);
        glUniform1i(data_uniform, 0);
        glUniformMatrix4fv(transform_uniform, 1, false, (float *) &camera);

        // bind_vertex_and_index_buffers(vi_buffers[vi_id_cube].vb, vi_buffers[vi_id_cube].ib);
        // glDrawElements(GL_TRIANGLES, vi_buffers[vi_id_cube].n, GL_UNSIGNED_SHORT, 0);

        glPatchParameteri(GL_PATCH_VERTICES, 4);
        glBegin(GL_PATCHES);
        glVertexAttrib2f(attrib_tex_coord, 0.0, 0.0); glVertexAttrib3f(attrib_normal, 0.0, 0.0, 1.0); glVertex3f(-1, -1, 0);
        glVertexAttrib2f(attrib_tex_coord, 1.0, 0.0); glVertexAttrib3f(attrib_normal, 0.0, 0.0, 1.0); glVertex3f( 1, -1, 0);
        glVertexAttrib2f(attrib_tex_coord, 1.0, 1.0); glVertexAttrib3f(attrib_normal, 0.0, 0.0, 1.0); glVertex3f( 1,  1, 0);
        glVertexAttrib2f(attrib_tex_coord, 0.0, 1.0); glVertexAttrib3f(attrib_normal, 0.0, 0.0, 1.0); glVertex3f(-1,  1, 0);
        glEnd();

        //render fbo
        #if 1
        // glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        // glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
        // glDrawBuffer(GL_BACK);                       // Set the back buffer as the draw buffer

        // glBlitFramebuffer(0, 0, window_width, window_height,
        //                   0, 0, window_width, window_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        #else
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, window_width, window_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(post_program);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, colorbuffer);
        glUniform1i(post_tex_uniform, 0);

        glUniform1i(width_uniform, window_width);
        glUniform1i(height_uniform, window_height);

        glBegin(GL_TRIANGLE_FAN);

        glVertex2f( 1.0,  1.0);
        glVertex2f(-1.0,  1.0);
        glVertex2f(-1.0, -1.0);
        glVertex2f( 1.0, -1.0);

        glEnd();
        #endif
        SwapBuffers(dc);
    }
    while(msg.message != WM_QUIT);

    return 0;
}
