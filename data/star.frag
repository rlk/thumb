
varying vec3  color;
varying float s;

void main()
{
    vec2 d = gl_PointCoord - vec2(0.5);

    vec3 k = exp(-29.556 * dot(d, d) / color);

    gl_FragColor = vec4(k, 1.0);
}
