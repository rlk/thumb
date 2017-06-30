uniform sampler2D L_map;
uniform sampler2D R_map;

void main()
{
    vec4  L = texture2D(L_map, gl_TexCoord[0].st);
    vec4  R = texture2D(R_map, gl_TexCoord[0].st);

    float K = step(0.5, fract((gl_FragCoord.y - 0.5) * 0.5));

    gl_FragColor = mix(R, L, K);
}
