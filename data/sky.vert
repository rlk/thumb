
varying vec3 vertex;

void main()
{
    vertex      = gl_Vertex.xyz;
    gl_Position = ftransform();
}
