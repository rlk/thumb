varying vec3 ray;
uniform vec3 pos;

void main(void)
{
    ray = gl_MultiTexCoord0.xyz - pos;

    gl_Position = gl_Vertex;
}
