
varying vec3 normal;

void main()
{
    normal         = (gl_TextureMatrix[0] * vec4(gl_Normal, 0.0)).xyz;
    gl_TexCoord[0] = gl_MultiTexCoord0;
    gl_Position    = ftransform();
}
