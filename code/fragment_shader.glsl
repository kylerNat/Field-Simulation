#version 440

uniform sampler2D data;

smooth in vec2 uv;
smooth in vec3 norm;

void main()
{
    gl_FragColor.xyz = texture(data, uv).xyz;
    gl_FragColor.xyz += (1.0-gl_FragCoord.z);
    gl_FragColor.w = 1.0;
}
