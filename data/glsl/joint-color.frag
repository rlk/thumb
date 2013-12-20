
varying vec3 fV;
varying vec3 fN;

void main()
{
    vec3 V = -normalize(fV);
    vec3 N =  normalize(fN);
    vec3 H =  normalize(V + vec3(1.0));

    float k = max(dot(V, N), 0.0);
    float l = max(dot(H, N), 0.0);

    vec3 C = (1.0 - pow(l,  0.7)) * vec3(0.5, 0.5, 0.0)
           + (1.0 - pow(k,  0.5)) * vec3(0.0, 0.5, 0.5)
           + (      pow(l, 10.0)) * vec3(0.8, 0.8, 0.8)
           + (      pow(k,  2.0)) * vec3(0.8, 0.0, 0.8);

    gl_FragColor = vec4(C, 1.0);
}
