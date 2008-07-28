
varying vec3 color;

void main()
{
    vec2 d = gl_PointCoord - vec2(0.5);

    float k = (0.5 - length(d)) / 0.5;

    gl_FragColor = vec4(color * k, 1.0);
}
