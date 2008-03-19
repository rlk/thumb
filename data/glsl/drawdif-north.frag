#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect cyl;
uniform sampler2D     src;

uniform vec2 d;
uniform vec2 k;

void main()
{
    vec4 c = texture2DRect(cyl, gl_FragCoord.xy);
    c.w = -c.w;
    vec2 t = (c.zw + 1.0) * 0.5;

    vec2  a = step(vec2(0.0), t) * step(t, vec2(1.0));
    float k = a.x * a.y;

    gl_FragColor = vec4(texture2D(src, t).rgb, k);
}
