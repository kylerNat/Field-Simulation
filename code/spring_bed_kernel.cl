#define w 500
#define h 500

#define sq(a) ((a)*(a))

const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE|CLK_ADDRESS_CLAMP|CLK_FILTER_NEAREST;
#define psi(x, y) read_imagef(psi_data, sampler, (int2)(x, y)).x
#define psi_dot(x, y) read_imagef(psi_data, sampler, (int2)(x, y)).y

#define k 50.0
#define k_center 0.0

float psi_ddot(int x, int y, float dt, __read_write image2d_t psi_data, float psi_ddot)
{
    /* psi_ddot += -(k*4.8+k_center)*psi; */
    /* psi_ddot -= 0.001*2.5*psi_dot; */
    return psi_ddot;
}

__kernel void simulate(float dt, __read_only image2d_t psi_data, __write_only image2d_t psi_new_data)
{

    int x = get_global_id(0);
    int y = get_global_id(1);

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
                    psi_ddot_base += (psi(x+dx,y+dy)-psi(x, y))*c[1+dx+(1+dy)*3];
                }
            }

        #if 1
        float k0 = dt*psi_ddot(x, y, dt, psi_data, psi_ddot_base);
        float l0 = dt*psi_dot(x, y);

        float psi_dot_new = (k0)+psi_dot(x, y);
        float psi_new = 0.5f*(k0)*dt+(psi_dot_new*dt)+psi(x, y);

        float4 psi_new_color = (float4)(psi_new, psi_dot_new, 0, 1);
        write_imagef(psi_new_data, (int2)(x, y), psi_new_color);

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
