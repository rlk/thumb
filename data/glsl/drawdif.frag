#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect cyl;
uniform sampler2D     color;

void main()
{
    vec4 t = texture2DRect(cyl, gl_FragCoord.xy);
    vec4 c = texture2D(color, t.zw);

    gl_FragColor = c;
}
