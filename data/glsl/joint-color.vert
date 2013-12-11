
varying vec3 V_v;
varying vec3 N_v;
varying vec3 L_v;

uniform vec4 light_position;

void main()
{
    V_v = (gl_ModelViewMatrix * gl_Vertex).xyz;
    N_v =  gl_NormalMatrix * gl_Normal;
    L_v =  light_position.xyz;

    gl_Position = ftransform();
}
