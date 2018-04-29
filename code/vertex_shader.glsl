#version 440

layout (location = 0) in vec3 p;
layout (location = 1) in vec3 n;
layout (location = 2) in vec2 uv_in;

uniform mat4 t;

smooth out vec3 norm;
smooth out vec2 uv;

void main()
{
    gl_Position.xyz = p;
    gl_Position.w = 1.0;
    gl_Position = t*gl_Position;
    uv = uv_in;
    norm = (t*vec4(n, 1.0)).xyz;
}
