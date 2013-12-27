
varying vec3 fV;
varying vec3 fL;

void main()
{
    // Generate points on the far plane in clip coordinates.

    vec4 c = vec4(gl_MultiTexCoord0.xy, 0.999, 1.0);

    // This funky MVP inversion eliminates the view translation.

    fV = vec3(gl_ModelViewMatrixTranspose * gl_ProjectionMatrixInverse * c);

    // Light vector is given by the first directional light source position.

    fL = vec3(0.0, 1.0, 0.0);

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
