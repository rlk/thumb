
varying vec3 V_v;

void main()
{
    vec3 V = normalize(V_v);

    gl_FragColor = vec4(V * 0.5 + 0.5, 1.0);
}
