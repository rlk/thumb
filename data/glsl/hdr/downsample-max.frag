#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect src;

void main()
{
    vec2 t0 = 2.0 * gl_FragCoord.xy + vec2(-0.5, -0.5);
    vec2 t1 = 2.0 * gl_FragCoord.xy + vec2(-0.5, +0.5);
    vec2 t2 = 2.0 * gl_FragCoord.xy + vec2(+0.5, -0.5);
    vec2 t3 = 2.0 * gl_FragCoord.xy + vec2(+0.5, +0.5);

    gl_FragColor = max(max(texture2DRect(src, t0),
                           texture2DRect(src, t1)),
                       max(texture2DRect(src, t2),
                           texture2DRect(src, t3)));
}
