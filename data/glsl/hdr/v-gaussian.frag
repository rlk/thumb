#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect src;

vec4 cut(vec4 c)
{
    return c;
}

void main()
{
    /*
    const vec2 d0 = vec2(0.0, -2.0);
    const vec2 d1 = vec2(0.0, -1.0);
    const vec2 d2 = vec2(0.0, +0.0);
    const vec2 d3 = vec2(0.0, +1.0);
    const vec2 d4 = vec2(0.0, +2.0);

    vec4 c0 = cut(texture2DRect(src, gl_FragCoord.xy + d0)) * 0.061;
    vec4 c1 = cut(texture2DRect(src, gl_FragCoord.xy + d1)) * 0.242;
    vec4 c2 = cut(texture2DRect(src, gl_FragCoord.xy + d2)) * 0.383;
    vec4 c3 = cut(texture2DRect(src, gl_FragCoord.xy + d3)) * 0.242;
    vec4 c4 = cut(texture2DRect(src, gl_FragCoord.xy + d4)) * 0.061;

    gl_FragColor = (c0 + c1 + c2 + c3 + c4);
    */
    vec4 c =
        cut(texture2DRect(src, gl_FragCoord.xy + vec2(0.0, -4.0))) * 0.0267 +
        cut(texture2DRect(src, gl_FragCoord.xy + vec2(0.0, -3.0))) * 0.0648 +
        cut(texture2DRect(src, gl_FragCoord.xy + vec2(0.0, -2.0))) * 0.1210 +
        cut(texture2DRect(src, gl_FragCoord.xy + vec2(0.0, -1.0))) * 0.1760 +
        cut(texture2DRect(src, gl_FragCoord.xy + vec2(0.0,  0.0))) * 0.1995 +
        cut(texture2DRect(src, gl_FragCoord.xy + vec2(0.0, +1.0))) * 0.1760 +
        cut(texture2DRect(src, gl_FragCoord.xy + vec2(0.0, +2.0))) * 0.1210 +
        cut(texture2DRect(src, gl_FragCoord.xy + vec2(0.0, +3.0))) * 0.0648 +
        cut(texture2DRect(src, gl_FragCoord.xy + vec2(0.0, +4.0))) * 0.0267;

    gl_FragColor = vec4(c.rgb, 1.0);
}
