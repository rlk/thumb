
varying vec3 V_v;
varying vec3 T_v;
varying vec3 B_v;

uniform float reflection_cubemap_size;

//-----------------------------------------------------------------------------

//    d - c    Compute the solid angle subtended by the quad given by four
//    | \ |    UNIT vectors abcd.
//    a - b

float solid_angle(const vec3 a, const vec3 b, const vec3 c, const vec3 d)
{
    float ab = dot(a, b);
    float ad = dot(a, d);
    float bc = dot(b, c);
    float bd = dot(b, d);
    float cd = dot(c, d);

    return 2.0 * (atan(dot(a, cross(b, d)), 1.0 + ab + ad + bd) +
                  atan(dot(c, cross(d, b)), 1.0 + bc + cd + bd));
}

//-----------------------------------------------------------------------------

void main()
{
    float k = 0.5 / reflection_cubemap_size;

    vec3 a = normalize(V_v - T_v * k - B_v * k);
    vec3 b = normalize(V_v + T_v * k - B_v * k);
    vec3 c = normalize(V_v + T_v * k + B_v * k);
    vec3 d = normalize(V_v - T_v * k + B_v * k);
/*
    a = normalize(vec3( 1.0,  1.0,  1.0));
    b = normalize(vec3( 1.0, -1.0,  1.0));
    c = normalize(vec3(-1.0, -1.0,  1.0));
    d = normalize(vec3(-1.0,  1.0,  1.0));
*/
    float r = solid_angle(a, b, c, d);

    gl_FragColor = vec4(vec3(r), 1.0);
}
