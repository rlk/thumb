
attribute vec3 Tangent;

uniform mat4 view_M;
uniform mat4 view_I;

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

    V_v = (     V_e       * view_M).xyz;
    N_v = (vec4(N_e, 0.0) * view_M).xyz;
    T_v = (vec4(T_e, 0.0) * view_M).xyz;

    // Material and shadow map texture coordinates.

    gl_TexCoord[0] = gl_MultiTexCoord0;
    gl_TexCoord[1] = gl_TextureMatrix[1] * V_e;
    gl_TexCoord[2] = gl_TextureMatrix[2] * V_e;
    gl_TexCoord[3] = gl_TextureMatrix[3] * V_e;

    gl_Position = ftransform();
}
