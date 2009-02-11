
varying vec3 V_v;
varying vec3 N_v;
varying vec3 L_v;

void main()
{
    vec3  V  = normalize(V_v);
    vec3  N  = normalize(N_v);
    vec3  L  = normalize(L_v);
    vec3  R  = reflect(L, N);

    vec3  Ka = vec3(0.2, 0.2, 0.2);
    vec3  Kd = vec3(0.8, 0.0, 0.8);
    vec3  Ks = vec3(0.5, 0.5, 0.5);
    float Ns = 4.0;

    vec3 KS = pow(max(dot(V, R), 0.0), Ns) * Ks;
    vec3 KD =     max(dot(L, N), 0.0)      * Kd;
    vec3 KA =                                Ka;

    gl_FragColor = vec4(KS + KD + KA, 1.0);
}
