#version 120

attribute vec3 Tangent;

uniform mat4 ShadowMatrix[4];

varying vec3 fV;
varying vec3 fL[4];
varying vec3 fD[4];
varying vec4 fS[4];

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

    // Tangent-space light source vector

    fL[0] = T * normalize(mix(gl_LightSource[0].position.xyz,
                              gl_LightSource[0].position.xyz - e.xyz,
                              gl_LightSource[0].position.w));
    fL[1] = T * normalize(mix(gl_LightSource[1].position.xyz,
                              gl_LightSource[1].position.xyz - e.xyz,
                              gl_LightSource[1].position.w));
    fL[2] = T * normalize(mix(gl_LightSource[2].position.xyz,
                              gl_LightSource[2].position.xyz - e.xyz,
                              gl_LightSource[2].position.w));
    fL[3] = T * normalize(mix(gl_LightSource[3].position.xyz,
                              gl_LightSource[3].position.xyz - e.xyz,
                              gl_LightSource[3].position.w));

    // Tangent-space light source direction

    fD[0] = T * gl_LightSource[0].spotDirection;
    fD[1] = T * gl_LightSource[1].spotDirection;
    fD[2] = T * gl_LightSource[2].spotDirection;
    fD[3] = T * gl_LightSource[3].spotDirection;

    // Shadow map texture coordinates

    fS[0] = ShadowMatrix[0] * e;
    fS[1] = ShadowMatrix[1] * e;
    fS[2] = ShadowMatrix[2] * e;
    fS[3] = ShadowMatrix[3] * e;

    // Built-in vertex position and texture coordinate

    gl_TexCoord[0] = gl_MultiTexCoord0;
    gl_Position    = gl_ModelViewProjectionMatrix * gl_Vertex;
}
