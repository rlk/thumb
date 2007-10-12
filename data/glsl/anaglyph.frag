#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect L_map;
uniform sampler2DRect R_map;

uniform vec2 frag_k;
uniform vec2 frag_d;

void main()
{
    const vec4 luma = { 0.30, 0.59, 0.11, 0.00 };

    vec2 p = (gl_FragCoord.xy + frag_d) * frag_k;

    const vec4 L = texture2DRect(L_map, p);
    const vec4 R = texture2DRect(R_map, p);

//  gl_FragColor = vec4(dot(R, luma) * 0.36, 0.0, dot(L, luma), 1.0);
    gl_FragColor = vec4(dot(R, luma) * 0.72, 0.0, dot(L, luma), 1.0);
}
