
uniform vec4 LightUnit;
uniform vec4 LightPosition[4];

varying vec3  P;
varying vec3 fV;
varying vec3 fL;

const vec4 O = vec4(0.0, 0.0, 0.0, 1.0);

void main()
{
    // Compare this unit ID with the light unit IDs to determine light position.

    vec4 L;

    if      (LightUnit.x == gl_MultiTexCoord0.p) L = LightPosition[0];
    else if (LightUnit.y == gl_MultiTexCoord0.p) L = LightPosition[1];
    else if (LightUnit.z == gl_MultiTexCoord0.p) L = LightPosition[2];
    else if (LightUnit.w == gl_MultiTexCoord0.p) L = LightPosition[3];
    else                                         L = vec4(0.0, 1.0, 0.0, 0.0);

    // Generate points on the far plane in clip coordinates.

    vec4 c = vec4(gl_MultiTexCoord0.xy, 0.999, 1.0);

    // Compute the world-space view position, view vector, and light position.

     P = vec3(gl_ModelViewMatrixInverse   * O);
    fV = vec3(gl_ModelViewMatrixTranspose * gl_ProjectionMatrixInverse * c);
    fL = vec3(gl_ModelViewMatrixTranspose * L);

    gl_Position = c;
}
