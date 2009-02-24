#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect src;

vec4 cut(vec4 c)
{
    return step(1.0, c);
}

void main()
{
    const vec2 d0 = vec2(-2.0, 0.0);
    const vec2 d1 = vec2(-1.0, 0.0);
    const vec2 d2 = vec2(+0.0, 0.0);
    const vec2 d3 = vec2(+1.0, 0.0);
    const vec2 d4 = vec2(+2.0, 0.0);

    vec4 c0 = cut(texture2DRect(src, gl_FragCoord.xy + d0)) * 0.061;
    vec4 c1 = cut(texture2DRect(src, gl_FragCoord.xy + d1)) * 0.242;
    vec4 c2 = cut(texture2DRect(src, gl_FragCoord.xy + d2)) * 0.383;
    vec4 c3 = cut(texture2DRect(src, gl_FragCoord.xy + d3)) * 0.242;
    vec4 c4 = cut(texture2DRect(src, gl_FragCoord.xy + d4)) * 0.061;

    gl_FragColor = (c0 + c1 + c2 + c3 + c4);
}
