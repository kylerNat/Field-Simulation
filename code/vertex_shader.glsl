#version 440

layout (location = 0) in vec3 p;
//layout (location = 1) in vec3 n;

uniform mat4 t;

smooth out vec3 norm;

void main()
{
    gl_Position.xyz = p;
    gl_Position.w = 1.0;
    gl_Position = gl_Position*t;
    norm = gl_Position.xyz;
}
