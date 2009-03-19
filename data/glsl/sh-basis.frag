
uniform samplerCube Y[9];

varying vec3 V_v;
varying vec3 N_v;

void main()
{
    float x = V_v.x;

    float k = (textureCube(Y[0], N_v).r * step( 0.0, x) * step(x,  3.0) +
               textureCube(Y[1], N_v).r * step( 3.0, x) * step(x,  6.0) +
               textureCube(Y[2], N_v).r * step( 6.0, x) * step(x,  9.0) +
               textureCube(Y[3], N_v).r * step( 9.0, x) * step(x, 12.0) +
               textureCube(Y[4], N_v).r * step(12.0, x) * step(x, 15.0) +
               textureCube(Y[5], N_v).r * step(15.0, x) * step(x, 18.0) +
               textureCube(Y[6], N_v).r * step(18.0, x) * step(x, 21.0) +
               textureCube(Y[7], N_v).r * step(21.0, x) * step(x, 24.0) +
               textureCube(Y[8], N_v).r * step(24.0, x) * step(x, 27.0));

    gl_FragColor = vec4(vec3(0.0, 1.0, 0.0) * clamp(+k, 0.0, 1.0) +
                        vec3(1.0, 0.0, 0.0) * clamp(-k, 0.0, 1.0), 1.0);
}
