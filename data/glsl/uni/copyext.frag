#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect src;
uniform sampler2DRect pos;

void main()
{
    vec4 P = texture2DRect(pos, gl_FragCoord.xy);

    float rr = dot(P.xyz, P.xyz);

    gl_FragColor = vec4(rr, rr, 0.0, 1.0);
}
