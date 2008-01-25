#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect map;

uniform vec2 frag_k;
uniform vec2 frag_d;

void main()
{
    vec2 p = gl_FragCoord.xy * frag_k + frag_d;

    gl_FragColor = texture2DRect(map, p);
}
