varying vec3 fN;

void main()
{
	fN = gl_Normal;
    gl_Position = ftransform();
}
