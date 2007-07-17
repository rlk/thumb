varying vec3 normal;

void main()
{
    vec3 N = normalize(normal);

    gl_FragColor = vec4(gl_TexCoord[0].xy, N.xy);
}
