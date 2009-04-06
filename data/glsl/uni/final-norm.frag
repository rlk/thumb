#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect nrm;

void main()
{
    vec4 N = texture2DRect(nrm, gl_FragCoord.xy);

    gl_FragColor = vec4(N.rgb, 1.0);
}
