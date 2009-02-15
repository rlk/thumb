
varying vec3 V_v;
varying vec3 L_v;

uniform vec3 light;

void main()
{
    V_v = gl_Vertex.xyz;
    L_v = normalize(light);

    gl_Position = ftransform();
}
