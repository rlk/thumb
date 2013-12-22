varying vec3 fN;

void main()
{
    vec3 N = normalize(fN);

    vec4 C0 = vec4(0.0, 0.0, 0.0, 1.0);
    vec4 C1 = gl_LightSource[0].diffuse;

    float d = dot(N, gl_LightSource[0].spotDirection);
    float s =   step(gl_LightSource[0].spotCosCutoff, d)
            * pow(d, gl_LightSource[0].spotExponent);

    gl_FragColor = mix(C0, C1, s);
}
