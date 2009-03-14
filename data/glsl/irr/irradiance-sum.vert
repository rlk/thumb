
uniform vec2 siz;

void main()
{
    gl_Position = vec4(-1.0 + siz + siz * gl_Vertex.xy, 0.0, 1.0);
}
