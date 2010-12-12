#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect L_map;
uniform sampler2DRect R_map;

uniform vec2 frag_k;
uniform vec2 frag_d;

void main()
{
    vec2  p = gl_FragCoord.xy * frag_k + frag_d;

    vec4  L = texture2DRect(L_map, p);
    vec4  R = texture2DRect(R_map, p);

    float K = step(0.5, fract((gl_FragCoord.y - 0.5) * 0.5));

    gl_FragColor = mix(R, L, K);
}
