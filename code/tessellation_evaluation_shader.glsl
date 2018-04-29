#version 440

layout(quads, equal_spacing, ccw) in;

uniform sampler2D data;

patch smooth in vec3 patch_norm;
smooth out vec3 norm;
smooth in vec2 patch_uv[];
smooth out vec2 uv;

#define interpolate(v, suffix) mix(mix(v[0]suffix, v[1]suffix, gl_TessCoord.x), \
                                   mix(v[3]suffix, v[2]suffix, gl_TessCoord.x), \
                                   gl_TessCoord.y);

void main()
{
    gl_Position = interpolate(gl_in, .gl_Position);
    norm = patch_norm;
    uv = interpolate(patch_uv, );
    vec4 data_val = texture(data, uv);
    gl_Position.xyz -= 0.5*length(data_val.xy)*norm;
}
