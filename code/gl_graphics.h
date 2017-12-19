#ifndef GRAPHICS
#define GRAPHICS

#include <gl/gl.h>
#include "misc.h"

enum attrib
{
    attrib_pos,
    attrib_count,
};

struct vi_buffer
{
    uint vb;
    uint ib;
    size_t n;
};

#define DEBUG_SEVERITY_HIGH                           0x9146
#define DEBUG_SEVERITY_MEDIUM                         0x9147
#define DEBUG_SEVERITY_LOW                            0x9148

void APIENTRY gl_error_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char* message, const void* userParam)
{
    switch(severity)
    {
        case DEBUG_SEVERITY_HIGH:
            assert(0, "\nHIGH:", message);
            break;
        case DEBUG_SEVERITY_MEDIUM:
            log_warning("\nMEDIUM:");
            break;
        case DEBUG_SEVERITY_LOW:
            log_warning("\nLOW:");
            break;
        default:
            log_output("\nNOTICE:");
            break;
    }
    log_output(" ", message, "\n\n");
}

vi_buffer create_vertex_and_index_buffer(uint vb_size, float * vb_data, uint ib_size, uint16 * ib_data)
{
    vi_buffer out = {};
    glGenBuffers(1, &out.vb);//maybe just do this in bulk?
    glBindBuffer(GL_ARRAY_BUFFER, out.vb);
    glBufferData(GL_ARRAY_BUFFER, vb_size, vb_data, GL_STATIC_DRAW);//TODO: use glNamedBufferData if available

    glGenBuffers(1, &out.ib);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, out.ib);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, ib_size, ib_data, GL_STATIC_DRAW);

    out.n = ib_size/sizeof(ib_data[0]);

    return out;
}

enum vi_id
{
    vi_id_cube,
    vi_id_count,
};

struct all_vi_buffers
{
    vi_buffer * buffers;
    size_t n;
};

inline all_vi_buffers init_vertex_and_index_buffers(void * memory)
{
    all_vi_buffers out = {};
    float vertex_buffer[] = {
        +0.5, +0.5, +0.5, //0
        +0.5, +0.5, -0.5, //1
        +0.5, -0.5, +0.5, //2
        +0.5, -0.5, -0.5, //3
        -0.5, +0.5, +0.5, //4
        -0.5, +0.5, -0.5, //5
        -0.5, -0.5, +0.5, //6
        -0.5, -0.5, -0.5, //7
    };

    uint16 index_buffer[] = {
        0, 1, 2, 1, 3, 2,
        0, 4, 1, 1, 4, 5,
        0, 2, 4, 2, 6, 4,
        7, 5, 6, 6, 5, 4,
        7, 6, 3, 6, 2, 3,
        7, 3, 5, 5, 3, 1,
    };

    out.buffers = (vi_buffer*) memory;
    out.buffers[out.n++] = create_vertex_and_index_buffer(sizeof(vertex_buffer), vertex_buffer,
                                                          sizeof(index_buffer), index_buffer);

    return out;
}

//take a vi_buffer?? mabye
inline void bind_vertex_and_index_buffers(uint vb, uint ib)
{

    glBindBuffer(GL_ARRAY_BUFFER, vb);
    glEnableVertexAttribArray(attrib_pos);
    glVertexAttribPointer(attrib_pos, 3, GL_FLOAT, GL_FALSE, 12, 0);
    //glEnableVertexAttribArray(attrib_norm);
    //glVertexAttribPointer(attrib_norm, 3, GL_FLOAT, GL_FALSE, 24, (void *)12);

    /* glVertexAttribFormat(attrib_pos, 3, GL_FLOAT, GL_FALSE, 0); */
    /* //glVertexAttribFormat(attrib_norm, 3, GL_FLOAT, GL_FALSE, 12); */
    /* glVertexAttribBinding(attrib_pos, 0); */
    /* //glVertexAttribBinding(attrib_norm, 0); */
    /* glBindVertexBuffer(0, vb, 0, 12); */

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
}

GLuint init_shader(char * shader_source, GLenum shader_type)
{
    GLuint shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, &shader_source, 0);
    glCompileShader(shader);

    int error;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &error);
    #ifdef DEBUG //TODO?: use logging
    if(error == 0)
    {
        char info_log[128];
        glGetShaderInfoLog(shader, sizeof(info_log), 0, info_log);
        assert(0);
    }
    #endif

    return shader;
}

GLuint init_program(char* vertex_shader_source, char* fragment_shader_source)
{
    GLuint vertex_shader = init_shader(vertex_shader_source, GL_VERTEX_SHADER);
    GLuint fragment_shader = init_shader(fragment_shader_source, GL_FRAGMENT_SHADER);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);

    glLinkProgram(program);

    int error;
    glGetProgramiv(program, GL_LINK_STATUS, &error);
    #ifdef DEBUG
    if(error == 0)
    {
        char info_log[128];
        glGetProgramInfoLog(program, sizeof(info_log), 0, info_log);
        assert(0);
    }
    #endif

    glDetachShader(program, vertex_shader);
    glDetachShader(program, fragment_shader);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    return program;
}

#endif
