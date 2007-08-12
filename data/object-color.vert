
attribute vec3 Tangent;

varying vec4 eye;
varying vec4 vertex;
varying vec3 normal;
varying vec3 tangent;
varying vec3 bitangent;

varying vec3  V;
varying vec3  L;
varying float G;

void main()
{
    eye = gl_ModelViewMatrix * gl_Vertex;

    vec3 N = normalize(gl_NormalMatrix * gl_Normal);
    vec3 T = normalize(gl_NormalMatrix * Tangent);
    vec3 B = cross(N, T);

    L.x = dot(gl_LightSource[0].position.xyz, T);
    L.y = dot(gl_LightSource[0].position.xyz, B);
    L.z = dot(gl_LightSource[0].position.xyz, N);
    L   = normalize(L);

    V.x = dot(eye.xyz, T);
    V.y = dot(eye.xyz, B);
    V.z = dot(eye.xyz, N);
    V   = normalize(V);

    G = dot(N, gl_LightSource[0].position.xyz);

    gl_TexCoord[0] = gl_MultiTexCoord0;
    gl_TexCoord[1] = gl_TextureMatrix[1] * eye;
    gl_TexCoord[2] = gl_TextureMatrix[2] * eye;
    gl_TexCoord[3] = gl_TextureMatrix[3] * eye;

    gl_Position = ftransform();
}
