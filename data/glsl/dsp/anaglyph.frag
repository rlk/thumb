#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect L_map;
uniform sampler2DRect R_map;

uniform vec2 frag_k;
uniform vec2 frag_d;

uniform vec4 luma;

void main()
{
    vec2 p = gl_FragCoord.xy * frag_k + frag_d;

    const vec4 L = texture2DRect(L_map, p);
    const vec4 R = texture2DRect(R_map, p);

    const float l = 2.0 * dot(L, luma);
    const float r = 2.0 * dot(R, luma) * 0.3 / 0.7;

    gl_FragColor = vec4(l, r, r, 1.0);
}
