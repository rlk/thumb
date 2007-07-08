varying vec4 pos;

void main()
{
    pos = gl_Vertex;

    gl_TexCoord[0] = gl_MultiTexCoord0;
    gl_Position    = ftransform();
}
