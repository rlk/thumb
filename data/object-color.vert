
attribute vec3 Tangent;

varying vec4 vertex;
varying vec3 normal;
varying vec3 tangent;
varying vec3 bitangent;

varying vec3 V;
varying vec3 L;

void main()
{
    vec4 eye = gl_ModelViewMatrix * gl_Vertex;
    vec3 lit = gl_LightSource[0].position.xyz;

    vec3 N = normalize(gl_NormalMatrix * gl_Normal);
    vec3 T = normalize(gl_NormalMatrix * Tangent);
    vec3 B = cross(N, T);

    L.x = dot(lit, T);
    L.y = dot(lit, B);
    L.z = dot(lit, N);
    L = normalize(L);

    V.x = dot(eye.xyz, T);
    V.y = dot(eye.xyz, B);
    V.z = dot(eye.xyz, N);
    V = normalize(V);

    gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
    gl_TexCoord[1] = gl_TextureMatrix[1] * eye;
    gl_TexCoord[2] = gl_TextureMatrix[2] * eye;
    gl_TexCoord[3] = gl_TextureMatrix[3] * eye;

    gl_Position = ftransform();
}
