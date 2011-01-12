
void main()
{
    float kd = max(0.2, (gl_NormalMatrix * gl_Normal).y);
    gl_FrontColor = gl_Color * kd;
    gl_Position = ftransform();
}
