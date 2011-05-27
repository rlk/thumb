#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect src;

uniform vec2 window;
uniform vec2 size;

void main()
{
    vec2 t = (gl_FragCoord.xy - 0.5 * window + size) * 0.5;
    vec2 a = step(vec2(0.0), t) * step(t, size);

    float K = a.x * a.y;

    const float pi = 3.14159265358979323846;

    const vec2 ck = vec2(0.5, 1.0) / pi;
    const vec2 cd = vec2(0.5, 0.5);

    gl_FragColor = vec4(texture2DRect(src, t).xy * ck + cd, 0.0, K);
}
