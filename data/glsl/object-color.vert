#version 120

attribute vec3 Tangent;

uniform mat4 ShadowMatrix[4];

varying vec4 fV;
varying vec4 fN;
varying vec4 fL[4];
varying vec3 fD[4];
varying vec4 fS[4];

void main()
{
    // Calculate the tangent space transform and its inverse.

    vec3 t = normalize(gl_NormalMatrix * Tangent);
    vec3 n = normalize(gl_NormalMatrix * gl_Normal);

    mat3 I = mat3(t, cross(n, t), n);
    mat4 M = transpose(mat4(I));

    // Tangent-space fragment position.

    fV    = M * gl_ModelViewMatrix * gl_Vertex;

    // Tangent-space light source position

    fL[0] = M * gl_LightSource[0].position;
    fL[1] = M * gl_LightSource[1].position;
    fL[2] = M * gl_LightSource[2].position;
    fL[3] = M * gl_LightSource[3].position;

    // Tangent-space light source direction

    fD[0] = I * gl_LightSource[0].spotDirection;
    fD[1] = I * gl_LightSource[1].spotDirection;
    fD[2] = I * gl_LightSource[2].spotDirection;
    fD[3] = I * gl_LightSource[3].spotDirection;

    // Shadow map texture coordinates

    vec4 e = gl_ModelViewMatrix * gl_Vertex;

    fS[0] = ShadowMatrix[0] * e;
    fS[1] = ShadowMatrix[1] * e;
    fS[2] = ShadowMatrix[2] * e;
    fS[3] = ShadowMatrix[3] * e;

    // Built-in vertex position and texture coordinate

    gl_TexCoord[0] = gl_MultiTexCoord0;
    gl_Position    = gl_ModelViewProjectionMatrix * gl_Vertex;
}
