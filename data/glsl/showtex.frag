#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect src;

void main()
{
    gl_FragColor = texture2DRect(src, gl_FragCoord.xy);
}
