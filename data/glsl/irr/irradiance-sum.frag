
uniform samplerRect src;

void main()
{
    gl_FragColor =
        (textureRect(src, 2.0 * gl_FragCoord.xy + vec2(-0.5, -0.5)) +
         textureRect(src, 2.0 * gl_FragCoord.xy + vec2(-0.5, +0.5)) +
         textureRect(src, 2.0 * gl_FragCoord.xy + vec2(+0.5, -0.5)) +
         textureRect(src, 2.0 * gl_FragCoord.xy + vec2(+0.5, +0.5)));
}
