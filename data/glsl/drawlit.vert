
varying vec3 L;

void main(void)
{
    L = normalize(gl_LightSource[0].position.xyz);
    gl_Position = ftransform();
}
