#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect cyl;
uniform sampler2D     src;

uniform vec2 d;
uniform vec2 k;

void main()
{
    vec2 c = texture2DRect(cyl, gl_FragCoord.xy).xy;
    vec2 t = c * k + d;

    float q = 1.0 - abs(sin(c.y));
    q = q * q * (3.0 - 2.0 * q);

    vec2  a = step(vec2(0.0), t) * step(t, vec2(1.0));
    float K = a.x * a.y;

    gl_FragColor = vec4(texture2D(src, t).rgb * K * q, K * q);
}
