#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect map;
uniform vec2 k;
uniform vec2 d;

void main()
{
    gl_FragColor = texture2DRect(map, (gl_FragCoord.xy + d) * k);
}
