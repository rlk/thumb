
varying vec3 V_v;
varying vec3 L_v;

uniform vec3  light;
uniform float time;

void main()
{
    V_v = normalize(gl_Normal.xyz);
    L_v = normalize(light);

    gl_Position = gl_Vertex;
}