
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

    // Material and shadow map texture coordinates.

    gl_TexCoord[0] = gl_Vertex;
    gl_TexCoord[1] = shadow_matrix[0] * V_e;
    gl_TexCoord[2] = shadow_matrix[1] * V_e;
    gl_TexCoord[3] = shadow_matrix[2] * V_e;

    gl_Position = ftransform();
}
