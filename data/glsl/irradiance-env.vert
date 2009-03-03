
attribute vec3 Tangent;

uniform mat4 shadow_matrix[3];

uniform mat4 view_matrix;
uniform mat4 view_inverse;

varying vec3 V_v;
varying vec3 T_v;
varying vec3 N_v;

void main()
{
    // Compute the view, normal, and tangent eye-space vectors.

    vec4 V_e = gl_ModelViewMatrix * gl_Vertex;
    vec3 N_e = gl_NormalMatrix    * gl_Normal;
    vec3 T_e = gl_NormalMatrix    * Tangent;

    // Compute the view, normal, and tangent world-space varyings.

    V_v = (     V_e       * view_matrix).xyz;
    N_v = (vec4(N_e, 0.0) * view_matrix).xyz;
    T_v = (vec4(T_e, 0.0) * view_matrix).xyz;

    // Material and shadow map texture coordinates.

    gl_TexCoord[0] = gl_MultiTexCoord0;
    gl_TexCoord[1] = shadow_matrix[0] * V_e;
    gl_TexCoord[2] = shadow_matrix[1] * V_e;
    gl_TexCoord[3] = shadow_matrix[2] * V_e;

    gl_Position = ftransform();
}
