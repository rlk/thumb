varying vec3 fN;

void main()
{
	fN = gl_NormalMatrix * gl_Normal;
    gl_Position = ftransform();
}
