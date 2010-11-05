
uniform sampler2D color;

void main()
{
    vec4 p = texture2D(color, gl_TexCoord[0].xy);
    vec4 c = vec4(gl_Color.rgb, 1.0);
    
    gl_FragColor = p * c;
}
