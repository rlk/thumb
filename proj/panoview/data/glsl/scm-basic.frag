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

uniform scm color;

uniform vec2 A[16];
uniform vec2 B[16];

//------------------------------------------------------------------------------

vec4 add(vec4 a, vec4 b)
{
    return mix(a, b, b.a);
}

vec4 sample_color(vec2 t)
{
    return add(add(add(add(texture2D(color.S, (t * A[ 0] + B[ 0]) * color.r + color.b[ 0]) * vec4(1.0, 1.0, 1.0, color.a[ 0]),
                           texture2D(color.S, (t * A[ 1] + B[ 1]) * color.r + color.b[ 1]) * vec4(1.0, 1.0, 1.0, color.a[ 1])),
                       add(texture2D(color.S, (t * A[ 2] + B[ 2]) * color.r + color.b[ 2]) * vec4(1.0, 1.0, 1.0, color.a[ 2]),
                           texture2D(color.S, (t * A[ 3] + B[ 3]) * color.r + color.b[ 3]) * vec4(1.0, 1.0, 1.0, color.a[ 3]))),
                   add(add(texture2D(color.S, (t * A[ 4] + B[ 4]) * color.r + color.b[ 4]) * vec4(1.0, 1.0, 1.0, color.a[ 4]),
                           texture2D(color.S, (t * A[ 5] + B[ 5]) * color.r + color.b[ 5]) * vec4(1.0, 1.0, 1.0, color.a[ 5])),
                       add(texture2D(color.S, (t * A[ 6] + B[ 6]) * color.r + color.b[ 6]) * vec4(1.0, 1.0, 1.0, color.a[ 6]),
                           texture2D(color.S, (t * A[ 7] + B[ 7]) * color.r + color.b[ 7]) * vec4(1.0, 1.0, 1.0, color.a[ 7])))),
               add(add(add(texture2D(color.S, (t * A[ 8] + B[ 8]) * color.r + color.b[ 8]) * vec4(1.0, 1.0, 1.0, color.a[ 8]),
                           texture2D(color.S, (t * A[ 9] + B[ 9]) * color.r + color.b[ 9]) * vec4(1.0, 1.0, 1.0, color.a[ 9])),
                       add(texture2D(color.S, (t * A[10] + B[10]) * color.r + color.b[10]) * vec4(1.0, 1.0, 1.0, color.a[10]),
                           texture2D(color.S, (t * A[11] + B[11]) * color.r + color.b[11]) * vec4(1.0, 1.0, 1.0, color.a[11]))),
                   add(add(texture2D(color.S, (t * A[12] + B[12]) * color.r + color.b[12]) * vec4(1.0, 1.0, 1.0, color.a[12]),
                           texture2D(color.S, (t * A[13] + B[13]) * color.r + color.b[13]) * vec4(1.0, 1.0, 1.0, color.a[13])),
                       add(texture2D(color.S, (t * A[14] + B[14]) * color.r + color.b[14]) * vec4(1.0, 1.0, 1.0, color.a[14]),
                           texture2D(color.S, (t * A[15] + B[15]) * color.r + color.b[15]) * vec4(1.0, 1.0, 1.0, color.a[15])))));
}

//------------------------------------------------------------------------------

void main()
{
    vec4 k = sample_color(gl_TexCoord[0].xy);
    gl_FragColor = vec4(mix(vec3(color.k0), vec3(color.k1), vec3(k)), 1.0);
}
