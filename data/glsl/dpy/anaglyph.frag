uniform sampler2D L_map;
uniform sampler2D R_map;

uniform vec4 luma;

void main()
{
    vec4 L = texture2D(L_map, gl_TexCoord[0].st);
    vec4 R = texture2D(R_map, gl_TexCoord[0].st);

    float l = 1.0 * dot(L, luma);
    float r = 1.0 * dot(R, luma) * 0.3 / 0.7;

    gl_FragColor = vec4(l, r, r, 1.0);
}
