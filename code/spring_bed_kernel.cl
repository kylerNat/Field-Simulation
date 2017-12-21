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
                psi_ddot_base += (psi(x+dx,y+dy)-psi(x, y))*c[1+dx+(1+dy)*3];
            }

        float k0 = dt*psi_ddot(x, y, dt, psi_data, psi_ddot_base);
        float l0 = dt*psi_dot(x, y);

        float psi_dot_new = (k0)+psi_dot(x, y);
        float psi_new = 0.5f*(k0)*dt+(psi_dot_new*dt)+psi(x, y);

        float4 psi_new_color = (float4)(psi_new, psi_dot_new, 0, 1);
        write_imagef(psi_new_data, (int2)(x, y), psi_new_color);
    }
}
