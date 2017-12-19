#define w 200
#define h 200

#define sq(a) ((a)*(a))

typedef float real;

__kernel void simulate(__global real* psi_r,
                       __global real* psi_i,
                       __global real* psi_r_new,
                       __global real* psi_i_new,
                       real dt)
{
    int x = get_global_id(0);
    int y = get_global_id(1);
    
    int i = x+y*w;
    if(x > 0 && x < w-1 && y > 0 && y < h-1)
    {
        //const real laplacian[] =  {
        //    0.25, 0.50, 0.25,
        //    0.50, -3.0, 0.50,
        //    0.25, 0.50, 0.25,
        //};
        const real laplacian[] =  { 
           0.0, 0.5, 0.0, 
           0.5,-2.0, 0.5, 
           0.0, 0.5, 0.0, 
        };
        real dpsi_r_dt = 0.0;
        real dpsi_i_dt = 0.0;
        for(int dx = -1; dx <= 1; dx++)
            for(int dy = -1; dy <= 1; dy++)
            {
                dpsi_r_dt += -0.5f*laplacian[1+dx+(1+dy)*3]*psi_i[(x+dx)+(y+dy)*w];
                dpsi_i_dt += +0.5f*laplacian[1+dx+(1+dy)*3]*psi_r[(x+dx)+(y+dy)*w];
            }
        for(int dx = -1; dx <= 1; dx++)
            for(int dy = -1; dy <= 1; dy++)
            {
                dpsi_r_dt += -0.5f*laplacian[1+2*dx+(1+2*dy)*3]*psi_i[(x+2*dx)+(y+2*dy)*w];
                dpsi_i_dt += +0.5f*laplacian[1+2*dx+(1+2*dy)*3]*psi_r[(x+2*dx)+(y+2*dy)*w];
            }
#if 0
        if(x!=0.5f*w || y!=0.5f*h)
        {
            dpsi_r_dt += +10.0f*-pow((sq(x-0.5f*w)+sq(y-0.5f*h)), -0.5f)*psi_i[i]; 
            dpsi_i_dt += -10.0f*-pow((sq(x-0.5f*w)+sq(y-0.5f*h)), -0.5f)*psi_r[i]; 
        }
#elif 1
        dpsi_r_dt += +0.0001f*(sq(x-0.5f*w)+sq(y-0.5f*h))*psi_i[i]; 
        dpsi_i_dt += -0.0001f*(sq(x-0.5f*w)+sq(y-0.5f*h))*psi_r[i]; 
#else

#endif
        
        psi_r_new[i] = psi_r[i]+dt*dpsi_r_dt;
        psi_i_new[i] = psi_i[i]+dt*dpsi_i_dt;
    }
}
