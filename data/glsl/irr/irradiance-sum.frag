#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect src;

void main()
{
    gl_FragColor =
        (texture2DRect(src, 2.0 * gl_FragCoord.xy + vec2(-0.5, -0.5)) +
         texture2DRect(src, 2.0 * gl_FragCoord.xy + vec2(-0.5, +0.5)) +
         texture2DRect(src, 2.0 * gl_FragCoord.xy + vec2(+0.5, -0.5)) +
         texture2DRect(src, 2.0 * gl_FragCoord.xy + vec2(+0.5, +0.5)));
}
