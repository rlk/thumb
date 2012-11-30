uniform mat3 M;
uniform vec2 A[16];
uniform vec2 B[16];

uniform vec3  zoomv;
uniform float zoomk;

varying vec3 var_V;
varying vec3 var_L;

//------------------------------------------------------------------------------

float scale(float k, float t)
{
    if (k < 1.0)
        return min(t / k, 1.0 - (1.0 - t) * k);
    else
        return max(t / k, 1.0 - (1.0 - t) * k);
}

vec3 zoom(vec3 v)
{
    const float pi = 3.1415927;

    float a = acos(dot(v, zoomv));

    if (a > 0.0)
    {
        float b = scale(zoomk, a / pi) * pi;

        vec3 y = normalize(cross(v, zoomv));
        vec3 x = normalize(cross(zoomv, y));

        return zoomv * cos(b) + x * sin(b);
    }
    else return v;
}

vec3 scube(vec2 t)
{
    vec2  s = radians(t * 90.0 - 45.0);

    float x =  sin(s.x) * cos(s.y);
    float y = -cos(s.x) * sin(s.y);
    float z =  cos(s.x) * cos(s.y);

    return M * normalize(vec3(x, y, z));
}

//------------------------------------------------------------------------------

void main()
{
    vec3 v = zoom(scube(A[0] * gl_Vertex.xy + B[0]));

    var_L = gl_LightSource[0].position.xyz;
    var_V = v;

    gl_TexCoord[0].xy = gl_Vertex.xy;
    gl_Position = gl_ModelViewProjectionMatrix * vec4(v, 1.0);
}
