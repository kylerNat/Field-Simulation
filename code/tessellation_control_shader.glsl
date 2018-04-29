#version 440

layout(vertices = 4) out;

smooth in vec3 norm[];
patch smooth out vec3 patch_norm;
smooth in vec2 uv[];
smooth out vec2 patch_uv[];

void main()
{
    #define subdivision 500
    gl_TessLevelOuter[0] = subdivision;
    gl_TessLevelOuter[1] = subdivision;
    gl_TessLevelOuter[2] = subdivision;
    gl_TessLevelOuter[3] = subdivision;
    gl_TessLevelInner[0] = subdivision;
    gl_TessLevelInner[1] = subdivision;

    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    patch_uv[gl_InvocationID] = uv[gl_InvocationID];
    patch_norm = norm[gl_InvocationID];
}
