#version 440

smooth in vec3 norm;

void main()
{
    gl_FragColor.xyz = 0.5*(1.0-norm.z)*vec3(1.0);//(20.0-norm.z)*0.05*vec3(1.0);//(abs(dot(norm, vec3(1.0, 2.0, 10.0))) + 10.0)*0.03*vec3(1.0);
    gl_FragColor.w = 1.0;
}
