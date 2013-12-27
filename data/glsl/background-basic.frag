
varying vec4 fE;
varying vec4 fV;
varying vec3 fL;

void main()
{
    vec3 V = normalize(fV.xyz);
    vec3 L = normalize(fL);

    vec2   t = vec2(atan(V.x, V.z), asin(V.y));
    float dt = fwidth(t);

    vec2 e = vec2(0.95);
    vec2 c = smoothstep(e - dt, e + dt, fract(t * 36 / 6.283185));

    gl_FragColor = vec4(c, 0.0, 1.0);
}
