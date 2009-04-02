
varying vec3 k;

void main()
{
    k = gl_Vertex.xyz;

    gl_Position = ftransform();
}
