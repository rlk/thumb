uniform sampler2D cookie;

void main()
{
	gl_FragColor = texture2D(cookie, gl_TexCoord[0].xy);
}
