#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect cyl;
uniform sampler2D     color;

uniform vec2 coff;
uniform vec2 cscl;

void main()
{
    vec2 t = (texture2DRect(cyl, gl_FragCoord.xy).zw + coff) * cscl;

    vec4 c = texture2D(color, t);

    gl_FragColor = c;
}
