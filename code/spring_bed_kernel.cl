#define w 500
#define h 500

#define sq(a) ((a)*(a))

#define k 50.0
#define k_center 0.0

float psi_ddot(int x, int y, float dt, __global float* psi_, __global float* psi_dot_, float psi, float psi_dot, float psi_ddot)
{
    /* psi_ddot += -(k*4.8+k_center)*psi; */
    /* psi_ddot -= 0.001*2.5*psi_dot; */
    return psi_ddot;
}

__kernel void simulate(__global float* psi_, __global float* psi_dot_, __global float* psi_new, __global float* psi_dot_new,
                       float dt)
{
    int x = get_global_id(0);
    int y = get_global_id(1);

    #define psi psi_[i]        
    #define psi_dot psi_dot_[i]
        
    int i = x+y*w;
    if(x > 0 && x < w-1)
    {
        const float c[] = {
            k*0.2, k*1.0, k*0.2,
            k*1.0, k*0.0, k*1.0,
            k*0.2, k*1.0, k*0.2,
        };
        
        float psi_ddot_base = 0.0;
        for(int dx = -1; dx <= 1; dx++)
            for(int dy = -1; dy <= 1; dy++)
            {
                if(y+dy > 0 && y+dy < h-1)
                {
                    psi_ddot_base += (psi_[(x+dx)+(y+dy)*w]-psi)*c[1+dx+(1+dy)*3];
                }
            }
        
        #if 1
        float k0 = dt*psi_ddot(x, y, dt, psi_, psi_dot_, psi, psi_dot, psi_ddot_base);
        float l0 = dt*psi_dot;
        
        psi_dot_new[i] = (k0)+psi_dot;
        psi_new[i] = 0.5f*(k0)*dt+(psi_dot_new[i]*dt)+psi;
        
        #elif 0
        float k0 = dt*psi_ddot(x, y, dt, psi_, psi_dot_, psi, psi_dot, psi_ddot_base);
        float l0 = dt*psi_dot;
        float k1 = dt*psi_ddot(x, y, dt, psi_, psi_dot_, psi+l0, psi_dot+k0, psi_ddot_base);
        float l1 = dt*(psi_dot+k0);
        
        psi_dot += (k1);
        psi_new[i] = psi+(l1);
        #else
        float k0 = dt*psi_ddot(x, y, dt, psi_, psi_dot_, psi, psi_dot, psi_ddot_base);
        float l0 = dt*psi_dot;
        float k1 = dt*psi_ddot(x, y, dt, psi_, psi_dot_, psi+0.5*l0, psi_dot+0.5*k0, psi_ddot_base);
        float l1 = dt*(psi_dot+0.5*k0);
        float k2 = dt*psi_ddot(x, y, dt, psi_, psi_dot_, psi+0.5*l1, psi_dot+0.5*k1, psi_ddot_base);
        float l2 = dt*(psi_dot+0.5*k1);
        float k3 = dt*psi_ddot(x, y, dt, psi_, psi_dot_, psi+l2, psi_dot+k2, psi_ddot_base);
        float l3 = dt*(psi_dot+k2);
        
        psi_dot_new[i] += 1.0/6.0*(k0+2*k1+2*k2+k3);
        psi_new[i] = psi+1.0/6.0*(l0+2*l1+2*l2+l3);
        #endif
    }
}
