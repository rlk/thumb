#version 120

struct scm
{
    sampler2D S;
    vec2      r;
    vec2      b[16];
    float     a[16];
    float     k0;
    float     k1;
};

uniform scm height;

uniform mat3 M;
uniform vec2 A[16];
uniform vec2 B[16];

varying vec3 var_V;
varying vec3 var_L;

//------------------------------------------------------------------------------

vec4 sample_height(vec2 t)
{
    vec4   c = vec4(1.0);
    c = mix(c, texture2D(height.S, (t * A[ 0] + B[ 0]) * height.r + height.b[ 0]), 1.0);
    c = mix(c, texture2D(height.S, (t * A[ 1] + B[ 1]) * height.r + height.b[ 1]), height.a[ 1]);
    c = mix(c, texture2D(height.S, (t * A[ 2] + B[ 2]) * height.r + height.b[ 2]), height.a[ 2]);
    c = mix(c, texture2D(height.S, (t * A[ 3] + B[ 3]) * height.r + height.b[ 3]), height.a[ 3]);
    c = mix(c, texture2D(height.S, (t * A[ 4] + B[ 4]) * height.r + height.b[ 4]), height.a[ 4]);
    c = mix(c, texture2D(height.S, (t * A[ 5] + B[ 5]) * height.r + height.b[ 5]), height.a[ 5]);
    c = mix(c, texture2D(height.S, (t * A[ 6] + B[ 6]) * height.r + height.b[ 6]), height.a[ 6]);
    c = mix(c, texture2D(height.S, (t * A[ 7] + B[ 7]) * height.r + height.b[ 7]), height.a[ 7]);
    c = mix(c, texture2D(height.S, (t * A[ 8] + B[ 8]) * height.r + height.b[ 8]), height.a[ 8]);
    c = mix(c, texture2D(height.S, (t * A[ 9] + B[ 9]) * height.r + height.b[ 9]), height.a[ 9]);
    c = mix(c, texture2D(height.S, (t * A[10] + B[10]) * height.r + height.b[10]), height.a[10]);
    c = mix(c, texture2D(height.S, (t * A[11] + B[11]) * height.r + height.b[11]), height.a[11]);
    c = mix(c, texture2D(height.S, (t * A[12] + B[12]) * height.r + height.b[12]), height.a[12]);
    c = mix(c, texture2D(height.S, (t * A[13] + B[13]) * height.r + height.b[13]), height.a[13]);
    c = mix(c, texture2D(height.S, (t * A[14] + B[14]) * height.r + height.b[14]), height.a[14]);
    c = mix(c, texture2D(height.S, (t * A[15] + B[15]) * height.r + height.b[15]), height.a[15]);
    return c;
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
    float h = mix(height.k0, height.k1, k);
    vec3  v = h * scube(A[0] * gl_Vertex.xy + B[0]);

    var_L = gl_LightSource[0].position.xyz;
    var_V = v;

    gl_TexCoord[0].xy = gl_Vertex.xy;
    gl_Position       = gl_ModelViewProjectionMatrix * vec4(v, 1.0);
}
