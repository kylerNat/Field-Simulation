//TODO: add these from c code
#define w 500
#define h 500

#define sq(a) ((a)*(a))

const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE|CLK_ADDRESS_CLAMP|CLK_FILTER_NEAREST;

#define psii(x, y) read_imagef(psi_data, sampler, (int2)(x, y)).x
#define psir(x, y) read_imagef(psi_data, sampler, (int2)(x, y)).y

__kernel void simulate(float dt, __read_only image2d_t psi_data, __write_only image2d_t psi_new_data)
{
    int x = get_global_id(0);
    int y = get_global_id(1);

    int i = x+y*w;
    /* const float laplacian[] = { */
    /*     0.0, 0.5, 0.0, */
    /*     0.5,-2.0, 0.5, */
    /*     0.0, 0.5, 0.0, */
    /* }; */
    const float laplacian[] = {
        0.25, 0.5, 0.25,
        0.5, -3.0, 0.5,
        0.25, 0.5, 0.25,
    };

    float psi_dot_r = 0.0;
    float psi_dot_i = 0.0;
    for(int dx = -1; dx <= 1; dx++)
        for(int dy = -1; dy <= 1; dy++)
        {
            psi_dot_r += -psii(x+dx,y+dy)*laplacian[1+dx+(1+dy)*3];
            psi_dot_i += +psir(x+dx,y+dy)*laplacian[1+dx+(1+dy)*3];
        }

    float psir_new = psir(x, y) + dt*psi_dot_r;
    float psii_new = psii(x, y) + dt*psi_dot_i;

    float4 psi_new_color = (float4)(psir_new, psii_new, 0, 1);
    write_imagef(psi_new_data, (int2)(x, y), psi_new_color);
}
