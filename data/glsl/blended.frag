#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect L_map;

uniform vec2 frag_k;
uniform vec2 frag_d;

void main()
{
    vec2 p = (gl_FragCoord.xy + frag_d) * frag_k;

    const vec4 C = texture2DRect(L_map, p);

    gl_FragColor = vec4((C * gl_Color).rgb, 1.0);
}
