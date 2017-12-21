#version 440

uniform sampler2D data;

smooth in vec3 norm;

void main()
{
    gl_FragColor.xyz = texture(data, gl_FragCoord.xy/500.0/* norm.xy+vec2(0.5, 0.5) */).xzy;//0.5*(1.0-norm.z)*vec3(1.0);//(20.0-norm.z)*0.05*vec3(1.0);//(abs(dot(norm, vec3(1.0, 2.0, 10.0))) + 10.0)*0.03*vec3(1.0);
    gl_FragColor.w = 1.0;
}
