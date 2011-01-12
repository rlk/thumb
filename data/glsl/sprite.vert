
void main()
{
    float r = length(gl_ModelViewMatrix * gl_Vertex);
    
    gl_FrontColor = gl_Color;
    gl_PointSize  = 10.0 / r;
    gl_Position   = ftransform();
}
