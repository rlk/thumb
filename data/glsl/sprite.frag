
uniform sampler2D color;

void main()
{
    vec4 p = texture2D(color, gl_TexCoord[0].xy);
    vec4 c = gl_Color;
    
    gl_FragColor = p * c;
}
