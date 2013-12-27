
varying vec3 fV;
varying vec3 fL;

void main()
{
    vec3 V = normalize(fV);
    vec3 L = normalize(fL);

    vec4 a = vec4(0.4, 0.3, 0.2, 1.0);
    vec4 b = vec4(1.0, 1.0, 1.0, 1.0);
    vec4 c = vec4(1.0, 1.0, 1.0, 1.0);
    vec4 d = vec4(0.2, 0.5, 1.0, 1.0);

    float e = 0.05;

    gl_FragColor = mix(mix(b, a, pow(abs(V.y), 0.2)),
                       mix(c, d, pow(abs(V.y), 0.2)),
                       smoothstep(-e, +e, V.y));
}
