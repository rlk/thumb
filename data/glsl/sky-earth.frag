
varying vec3 fV;
varying vec3 fL;

void main()
{
    vec3 V = normalize(fV);
    vec3 L = normalize(fL);

    vec2 p = degrees(vec2(atan(V.x, V.z), asin(V.y)));

    vec2 c = step(0.9, fract(p / 10.0));

    float k = pow(dot(V, L), 50.0);

    gl_FragColor = vec4(c, k, 1.0);
}
