
void main()
{
    gl_TexCoord[0] = gl_MultiTexCoord0 * 2.0 - 1.0;
    gl_Position    = gl_Vertex;
}
