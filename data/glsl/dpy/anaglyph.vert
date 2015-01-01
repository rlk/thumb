void main(void)
{
	gl_TexCoord[0] = (gl_MultiTexCoord0 + 1.0f) / 2.0f;
    gl_Position    = ftransform();
}
