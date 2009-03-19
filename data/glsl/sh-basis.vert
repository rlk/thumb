uniform mat4 view_matrix;
uniform mat4 view_inverse;

varying vec3 V_v;
varying vec3 N_v;

void main()
{
    vec4 V_e =      gl_ModelViewMatrix * gl_Vertex;
    vec4 N_e = vec4(gl_NormalMatrix    * gl_Normal, 0.0);

    V_v = (gl_Vertex).xyz;
    N_v = (N_e * view_matrix).xyz;

    gl_Position = ftransform();
}
