
void main()
{
    float r = length(gl_ModelViewMatrix * gl_Vertex);
    
    gl_FrontColor = gl_Color;
    gl_PointSize  = 100.0 / r;
    gl_Position   = ftransform();
}
