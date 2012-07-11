#version 120

void main()
{
    gl_FragColor = vec4(gl_PointCoord.xy, 0.0, 1.0);
}
