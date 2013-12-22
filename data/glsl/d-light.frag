varying vec3 fN;

void main()
{
    vec3 N =  normalize(fN);
    vec3 L = -normalize(gl_LightSource[0].position.xyz);

    vec4 C0 = vec4(0.0, 0.0, 0.0, 1.0);
    vec4 C1 = gl_LightSource[0].diffuse;

    float d = dot(N, L);

    gl_FragColor = mix(C0, C1, d);
}
