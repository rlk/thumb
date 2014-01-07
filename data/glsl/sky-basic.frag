
varying vec3 fV;
varying vec3 fL;

void main()
{
    vec3 V = normalize(fV);
    vec3 L = normalize(fL);

    gl_FragColor = vec4(vec3((dot(V, L) + 1.0) / 2.0), 1.0);
}
