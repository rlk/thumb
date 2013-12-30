#version 120

attribute vec3 Tangent;

uniform vec4 LightPosition[4];
uniform mat4 ShadowMatrix[4];

varying vec3 fV;
varying vec3 fL[4];
varying vec4 fS[4];

vec3 calc_L(vec4 light, vec4 eye)
{
    return normalize(mix(light.xyz, light.xyz - eye.xyz, light.w));
}

void main()
{
    // Calculate the tangent space transform and inverse.

    vec3 t = normalize(gl_NormalMatrix * Tangent);
    vec3 n = normalize(gl_NormalMatrix * gl_Normal);

    mat3 I = mat3(t, cross(n, t), n);
    mat3 T = transpose(I);

    vec4 e = gl_ModelViewMatrix * gl_Vertex;

    // Tangent-space view vector

    fV = T * (-e.xyz);

    // Tangent-space light source vectors

    fL[0] = T * calc_L(LightPosition[0], e);
    fL[1] = T * calc_L(LightPosition[1], e);
    fL[2] = T * calc_L(LightPosition[2], e);
    fL[3] = T * calc_L(LightPosition[3], e);

    // Shadow map texture coordinates

    fS[0] = ShadowMatrix[0] * e;
    fS[1] = ShadowMatrix[1] * e;
    fS[2] = ShadowMatrix[2] * e;
    fS[3] = ShadowMatrix[3] * e;

    // Built-in vertex position and texture coordinate

    gl_TexCoord[0] = gl_MultiTexCoord0;
    gl_Position    = gl_ModelViewProjectionMatrix * gl_Vertex;
}
