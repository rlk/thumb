varying vec3 k;

void main()
{
    vec3 f  = fract (k * 100.0);
    vec3 df = fwidth(k * 100.0);

    vec3 g = smoothstep(df * 1.0, df * 2.0, f);

    float c = g.x * g.y * g.z;

    gl_FragColor = vec4(c, c, c, 1.0);
}
