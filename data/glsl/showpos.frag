#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect src;

uniform float range;

void main()
{
    gl_FragColor = (0.5 * (1.0 + texture2DRect(src, gl_FragCoord.xy) / range));
}
