
varying vec3 fV;
varying vec3 fN;

void main()
{
    fV = vec3(gl_ModelViewMatrix * gl_Vertex);
    fN = vec3(gl_NormalMatrix    * gl_Normal);

    gl_Position = ftransform();
}
