//
// Atmospheric scattering fragment shader
//
// Author: Sean O'Neil
//
// Copyright (c) 2004 Sean O'Neil
//

uniform mat4 eye_to_object_mat;
uniform mat4 eye_to_object_inv;
//uniform vec3 v3LightPos;
uniform float g;
uniform float g2;

varying vec3 v3Direction;
varying vec3 color_R;
varying vec3 color_M;

void main (void)
{
    vec3 v3LightPos = (eye_to_object_inv * gl_LightSource[0].position).xyz;

    float fCos = dot(v3LightPos, v3Direction) / length(v3Direction);

    float fMiePhase = 1.5 * ((1.0 - g2) / (2.0 + g2)) * (1.0 + fCos*fCos) / pow(1.0 + g2 - 2.0*g*fCos, 1.5);
    gl_FragColor.rgb = color_R + fMiePhase * color_M;
    gl_FragColor.a = gl_FragColor.b;
}
