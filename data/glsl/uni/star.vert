
uniform   vec3  Position;
uniform   float Multiplier;
attribute float Magnitude;

varying vec3  color;

void main()
{
    float d = length(gl_Vertex.xyz - Position);

    float m = Magnitude + 5.0 * log(d / 10.0) / log(10.0);

    float s = pow(10.0, -0.2 * m) * Multiplier;

    color = gl_Color.rgb;

    gl_Position  = ftransform();
//  gl_PointSize = s;
    gl_PointSize = 2.0;
}
