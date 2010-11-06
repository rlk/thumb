
void main()
{
    float r = length(gl_ModelViewMatrix * gl_Vertex);
    
    gl_FrontColor = mix(vec4(0.0), gl_Color, smoothstep(0.0, 0.1, gl_Vertex.y));
    gl_PointSize  = 50.0 / r;
    gl_Position   = ftransform();
}
