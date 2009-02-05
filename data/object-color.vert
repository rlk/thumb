
attribute vec3 Tangent;

varying vec3  V_v;
varying vec3  L_v;
/*
varying vec3  N_v;
varying vec3  T_v;
*/

void main()
{
    vec4 pos = gl_ModelViewMatrix * gl_Vertex;

    mat3 T;

    T[2] = normalize(gl_NormalMatrix * gl_Normal);
    T[0] = normalize(gl_NormalMatrix * Tangent);
    T[1] = normalize(cross(T[2], T[0]));

    L_v = (gl_LightSource[0].position.xyz) * T;
    V_v = pos.xyz * T;

    // Diffuse and shadow map texture coordinates.

    gl_TexCoord[0] = gl_MultiTexCoord0;
    gl_TexCoord[1] = gl_TextureMatrix[1] * pos;
    gl_TexCoord[2] = gl_TextureMatrix[2] * pos;
    gl_TexCoord[3] = gl_TextureMatrix[3] * pos;

    gl_Position = ftransform();
}
/*
void main()
{
    N_v = gl_NormalMatrix * gl_Normal;     // Eye space normal vector
    T_v = gl_NormalMatrix * Tangent;       // Eye space tangent vector
    L_v = gl_LightSource[0].position.xyz;  // Eye space light vector
    V_v = gl_ModelViewMatrix * gl_Vertex;  // Eye space view vector

    // Diffuse and shadow map texture coordinates.

    gl_TexCoord[0] = gl_MultiTexCoord0;
    gl_TexCoord[1] = gl_TextureMatrix[1] * V_v;
    gl_TexCoord[2] = gl_TextureMatrix[2] * V_v;
    gl_TexCoord[3] = gl_TextureMatrix[3] * V_v;

    gl_Position = ftransform();
}
*/
