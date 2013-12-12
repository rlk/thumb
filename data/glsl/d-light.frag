varying vec3 fN;

void main()
{
    vec3 N = normalize(fN);

    vec4 C0 = vec4(0.4, 0.4, 0.4, 1.0);
    vec4 C1 = gl_LightSource[0].diffuse;

    gl_FragColor = mix(C0, C1, step(0.9, -N.y));
}
