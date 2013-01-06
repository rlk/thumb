#version 120

varying vec3 var_V;
varying vec3 var_L;

struct scm
{
    sampler2D S;
    vec2      r;
    vec2      b[16];
    float     a[16];
    float     k0;
    float     k1;
};

uniform scm scalar;
uniform scm normal;

uniform vec2 A[16];
uniform vec2 B[16];

//------------------------------------------------------------------------------

vec4 add(vec4 a, vec4 b)
{
    return mix(a, b, b.a);
}

vec4 sample_scalar(vec2 t)
{
    return add(add(add(add(texture2D(scalar.S, (t * A[ 0] + B[ 0]) * scalar.r + scalar.b[ 0]) * vec4(1.0, 1.0, 1.0, scalar.a[ 0]),
                           texture2D(scalar.S, (t * A[ 1] + B[ 1]) * scalar.r + scalar.b[ 1]) * vec4(1.0, 1.0, 1.0, scalar.a[ 1])),
                       add(texture2D(scalar.S, (t * A[ 2] + B[ 2]) * scalar.r + scalar.b[ 2]) * vec4(1.0, 1.0, 1.0, scalar.a[ 2]),
                           texture2D(scalar.S, (t * A[ 3] + B[ 3]) * scalar.r + scalar.b[ 3]) * vec4(1.0, 1.0, 1.0, scalar.a[ 3]))),
                   add(add(texture2D(scalar.S, (t * A[ 4] + B[ 4]) * scalar.r + scalar.b[ 4]) * vec4(1.0, 1.0, 1.0, scalar.a[ 4]),
                           texture2D(scalar.S, (t * A[ 5] + B[ 5]) * scalar.r + scalar.b[ 5]) * vec4(1.0, 1.0, 1.0, scalar.a[ 5])),
                       add(texture2D(scalar.S, (t * A[ 6] + B[ 6]) * scalar.r + scalar.b[ 6]) * vec4(1.0, 1.0, 1.0, scalar.a[ 6]),
                           texture2D(scalar.S, (t * A[ 7] + B[ 7]) * scalar.r + scalar.b[ 7]) * vec4(1.0, 1.0, 1.0, scalar.a[ 7])))),
               add(add(add(texture2D(scalar.S, (t * A[ 8] + B[ 8]) * scalar.r + scalar.b[ 8]) * vec4(1.0, 1.0, 1.0, scalar.a[ 8]),
                           texture2D(scalar.S, (t * A[ 9] + B[ 9]) * scalar.r + scalar.b[ 9]) * vec4(1.0, 1.0, 1.0, scalar.a[ 9])),
                       add(texture2D(scalar.S, (t * A[10] + B[10]) * scalar.r + scalar.b[10]) * vec4(1.0, 1.0, 1.0, scalar.a[10]),
                           texture2D(scalar.S, (t * A[11] + B[11]) * scalar.r + scalar.b[11]) * vec4(1.0, 1.0, 1.0, scalar.a[11]))),
                   add(add(texture2D(scalar.S, (t * A[12] + B[12]) * scalar.r + scalar.b[12]) * vec4(1.0, 1.0, 1.0, scalar.a[12]),
                           texture2D(scalar.S, (t * A[13] + B[13]) * scalar.r + scalar.b[13]) * vec4(1.0, 1.0, 1.0, scalar.a[13])),
                       add(texture2D(scalar.S, (t * A[14] + B[14]) * scalar.r + scalar.b[14]) * vec4(1.0, 1.0, 1.0, scalar.a[14]),
                           texture2D(scalar.S, (t * A[15] + B[15]) * scalar.r + scalar.b[15]) * vec4(1.0, 1.0, 1.0, scalar.a[15])))));
}

vec4 sample_normal(vec2 t)
{
    return add(add(add(add(texture2D(normal.S, (t * A[ 0] + B[ 0]) * normal.r + normal.b[ 0]) * vec4(1.0, 1.0, 1.0, normal.a[ 0]),
                           texture2D(normal.S, (t * A[ 1] + B[ 1]) * normal.r + normal.b[ 1]) * vec4(1.0, 1.0, 1.0, normal.a[ 1])),
                       add(texture2D(normal.S, (t * A[ 2] + B[ 2]) * normal.r + normal.b[ 2]) * vec4(1.0, 1.0, 1.0, normal.a[ 2]),
                           texture2D(normal.S, (t * A[ 3] + B[ 3]) * normal.r + normal.b[ 3]) * vec4(1.0, 1.0, 1.0, normal.a[ 3]))),
                   add(add(texture2D(normal.S, (t * A[ 4] + B[ 4]) * normal.r + normal.b[ 4]) * vec4(1.0, 1.0, 1.0, normal.a[ 4]),
                           texture2D(normal.S, (t * A[ 5] + B[ 5]) * normal.r + normal.b[ 5]) * vec4(1.0, 1.0, 1.0, normal.a[ 5])),
                       add(texture2D(normal.S, (t * A[ 6] + B[ 6]) * normal.r + normal.b[ 6]) * vec4(1.0, 1.0, 1.0, normal.a[ 6]),
                           texture2D(normal.S, (t * A[ 7] + B[ 7]) * normal.r + normal.b[ 7]) * vec4(1.0, 1.0, 1.0, normal.a[ 7])))),
               add(add(add(texture2D(normal.S, (t * A[ 8] + B[ 8]) * normal.r + normal.b[ 8]) * vec4(1.0, 1.0, 1.0, normal.a[ 8]),
                           texture2D(normal.S, (t * A[ 9] + B[ 9]) * normal.r + normal.b[ 9]) * vec4(1.0, 1.0, 1.0, normal.a[ 9])),
                       add(texture2D(normal.S, (t * A[10] + B[10]) * normal.r + normal.b[10]) * vec4(1.0, 1.0, 1.0, normal.a[10]),
                           texture2D(normal.S, (t * A[11] + B[11]) * normal.r + normal.b[11]) * vec4(1.0, 1.0, 1.0, normal.a[11]))),
                   add(add(texture2D(normal.S, (t * A[12] + B[12]) * normal.r + normal.b[12]) * vec4(1.0, 1.0, 1.0, normal.a[12]),
                           texture2D(normal.S, (t * A[13] + B[13]) * normal.r + normal.b[13]) * vec4(1.0, 1.0, 1.0, normal.a[13])),
                       add(texture2D(normal.S, (t * A[14] + B[14]) * normal.r + normal.b[14]) * vec4(1.0, 1.0, 1.0, normal.a[14]),
                           texture2D(normal.S, (t * A[15] + B[15]) * normal.r + normal.b[15]) * vec4(1.0, 1.0, 1.0, normal.a[15])))));
}

//------------------------------------------------------------------------------

float peak(float k, float c)
{
    return max(0.0, 1.0 - abs(k - c) * 5.0);
}

vec3 colormap(float k)
{
    return peak(k, 0.0) * vec3(1.0, 0.0, 1.0) +
           peak(k, 0.2) * vec3(0.0, 0.0, 1.0) +
           peak(k, 0.4) * vec3(0.0, 1.0, 1.0) +
           peak(k, 0.6) * vec3(1.0, 1.0, 0.0) +
           peak(k, 0.8) * vec3(1.0, 0.0, 0.0) +
           peak(k, 1.0) * vec3(1.0, 1.0, 1.0);
}

//------------------------------------------------------------------------------

void main()
{
    vec3 V = normalize(var_V);
    vec3 L = normalize(var_L);

    float k = mix(scalar.k0, scalar.k1, sample_scalar(gl_TexCoord[0].xy).r);
    vec3  N = normalize(sample_normal(gl_TexCoord[0].xy).rgb * 2.0 - 1.0);

    float nl = max(0.0, dot(N, L));
    float nv = max(0.0, dot(N, V));
    float kd = 2.0 * nl / (nl + nv);

    gl_FragColor = vec4(colormap(k) * kd, 1.0);
}
