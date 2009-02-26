
uniform sampler2D diff_map;

void main()
{
    gl_FragColor = texture2D(diff_map, gl_TexCoord[0].xy);
}
