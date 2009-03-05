
varying vec3 V_v;
varying vec3 T_v;
varying vec3 B_v;

uniform float reflection_cubemap_size;

//-----------------------------------------------------------------------------

//    c - d    Compute the solid angle subtended by the quad given by four
//    | / |    UNIT vectors abcd.
//    a - b

float solid_angle(const vec3 a, const vec3 b, const vec3 c, const vec3 d)
{
    float ab = dot(a, b);
    float ac = dot(a, c);
    float ad = dot(a, d);
    float bd = dot(b, d);
    float cd = dot(c, d);

    return 2.0 * (atan(dot(a, cross(b, d)), 1.0 + ab + ad + bd) +
                  atan(dot(d, cross(c, a)), 1.0 + cd + ad + ac));
}

//-----------------------------------------------------------------------------

void main()
{
//  float k = 0.5 / reflection_cubemap_size;
    float k = 0.5 / 128.0;

    vec3 a = normalize(V_v - T_v * k - B_v * k);
    vec3 b = normalize(V_v + T_v * k - B_v * k);
    vec3 c = normalize(V_v - T_v * k + B_v * k);
    vec3 d = normalize(V_v + T_v * k + B_v * k);

    gl_FragColor = vec4(vec3(solid_angle(a, b, c, d)), 1.0);
}
