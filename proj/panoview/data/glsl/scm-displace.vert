#version 120

struct scm
{
    sampler2DRect img;
    vec2          A;
    vec2          B[16];
    vec4          K[16];
};

uniform scm height;

uniform mat3 M;
uniform vec2 A[16];
uniform vec2 B[16];

uniform float r0;
uniform float r1;

varying vec3 var_V;
varying vec3 var_L;

//------------------------------------------------------------------------------

vec4 add(vec4 a, vec4 b)
{
    return mix(a, b, b.a);
}

vec4 sample_height(vec2 t)
{
    return add(add(add(add(texture2DRect(height.img, (t * A[ 0] + B[ 0]) * height.A + height.B[ 0] + 1.0) * height.K[ 0],
                           texture2DRect(height.img, (t * A[ 1] + B[ 1]) * height.A + height.B[ 1] + 1.0) * height.K[ 1]),
                       add(texture2DRect(height.img, (t * A[ 2] + B[ 2]) * height.A + height.B[ 2] + 1.0) * height.K[ 2],
                           texture2DRect(height.img, (t * A[ 3] + B[ 3]) * height.A + height.B[ 3] + 1.0) * height.K[ 3])),
                   add(add(texture2DRect(height.img, (t * A[ 4] + B[ 4]) * height.A + height.B[ 4] + 1.0) * height.K[ 4],
                           texture2DRect(height.img, (t * A[ 5] + B[ 5]) * height.A + height.B[ 5] + 1.0) * height.K[ 5]),
                       add(texture2DRect(height.img, (t * A[ 6] + B[ 6]) * height.A + height.B[ 6] + 1.0) * height.K[ 6],
                           texture2DRect(height.img, (t * A[ 7] + B[ 7]) * height.A + height.B[ 7] + 1.0) * height.K[ 7]))),
               add(add(add(texture2DRect(height.img, (t * A[ 8] + B[ 8]) * height.A + height.B[ 8] + 1.0) * height.K[ 8],
                           texture2DRect(height.img, (t * A[ 9] + B[ 9]) * height.A + height.B[ 9] + 1.0) * height.K[ 9]),
                       add(texture2DRect(height.img, (t * A[10] + B[10]) * height.A + height.B[10] + 1.0) * height.K[10],
                           texture2DRect(height.img, (t * A[11] + B[11]) * height.A + height.B[11] + 1.0) * height.K[11])),
                   add(add(texture2DRect(height.img, (t * A[12] + B[12]) * height.A + height.B[12] + 1.0) * height.K[12],
                           texture2DRect(height.img, (t * A[13] + B[13]) * height.A + height.B[13] + 1.0) * height.K[13]),
                       add(texture2DRect(height.img, (t * A[14] + B[14]) * height.A + height.B[14] + 1.0) * height.K[14],
                           texture2DRect(height.img, (t * A[15] + B[15]) * height.A + height.B[15] + 1.0) * height.K[15]))));
}

//------------------------------------------------------------------------------

float peak(float k, float c)
{
    return max(0.0, 1.0 - abs(k - c) * 5.0);
}

vec4 colormap(float k)
{
    return peak(k, 0.0) * vec4(1.0, 0.0, 1.0, 1.0) +
           peak(k, 0.2) * vec4(0.0, 0.0, 1.0, 1.0) +
           peak(k, 0.4) * vec4(0.0, 1.0, 1.0, 1.0) +
           peak(k, 0.6) * vec4(1.0, 1.0, 0.0, 1.0) +
           peak(k, 0.8) * vec4(1.0, 0.0, 0.0, 1.0) +
           peak(k, 1.0) * vec4(1.0, 1.0, 1.0, 1.0);
}

//------------------------------------------------------------------------------

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
    float k = sample_height(gl_Vertex.xy).r;
    float h = mix(r0, r1, k);
    vec3  v = h * scube(A[0] * gl_Vertex.xy + B[0]);

    var_L = gl_LightSource[0].position.xyz;
    var_V = v;

    gl_TexCoord[0].xy = gl_Vertex.xy;
    gl_FrontColor     = colormap(k);
    gl_Position       = gl_ModelViewProjectionMatrix * vec4(v, 1.0);
}
