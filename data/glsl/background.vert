
varying vec4 fE;
varying vec4 fV;
varying vec3 fL;

void main()
{
    vec4 c = vec4(gl_MultiTexCoord0.xy, 0.999, 1.0);

    fE = gl_ProjectionMatrixInverse * c;
    fV = fE * gl_ModelViewMatrix;
    fL = vec3(0.0, 1.0, 0.0);

    // Light vector is given by the first directional light source position.

    if      (gl_LightSource[0].position.w == 0.0)
        fL = gl_LightSource[0].position.xyz;
    else if (gl_LightSource[1].position.w == 0.0)
        fL = gl_LightSource[1].position.xyz;
    else if (gl_LightSource[2].position.w == 0.0)
        fL = gl_LightSource[2].position.xyz;
    else if (gl_LightSource[3].position.w == 0.0)
        fL = gl_LightSource[3].position.xyz;

    gl_Position = c;
}
