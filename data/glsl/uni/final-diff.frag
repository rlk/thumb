#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect cyl;
uniform sampler2DRect dif;
uniform sampler2DRect nrm;

void main()
{
    vec4 D = texture2DRect(dif, gl_FragCoord.xy);

    gl_FragColor = vec4(D.rgb, 1.0);
}
