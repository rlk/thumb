
attribute vec3 Tangent;

varying vec3 V_v;
varying vec3 L_v;

void main()
{
    vec4 eye = gl_ModelViewMatrix * gl_Vertex;

    mat3 T;

    T[2] = normalize(gl_NormalMatrix * gl_Normal);
    T[0] = normalize(gl_NormalMatrix * Tangent);
    T[1] = normalize(cross(T[2], T[0]));

    L_v = (gl_LightSource[0].position.xyz - eye.xyz) * T;
    V_v = eye.xyz * T;

    // Diffuse and shadow map texture coordinates.

    gl_TexCoord[0] = gl_MultiTexCoord0;
    gl_TexCoord[1] = gl_TextureMatrix[1] * eye;
    gl_TexCoord[2] = gl_TextureMatrix[2] * eye;
    gl_TexCoord[3] = gl_TextureMatrix[3] * eye;

    gl_Position = ftransform();
}