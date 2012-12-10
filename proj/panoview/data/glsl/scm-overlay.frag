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

uniform scm base;
uniform scm over;

uniform vec2 A[16];
uniform vec2 B[16];

//------------------------------------------------------------------------------

vec4 add(vec4 a, vec4 b)
{
    return mix(a, b, b.a);
}

vec4 sample_base(vec2 t)
{
    return add(add(add(add(texture2D(base.S, (t * A[ 0] + B[ 0]) * base.r + base.b[ 0]) * vec4(1.0, 1.0, 1.0, base.a[ 0]),
                           texture2D(base.S, (t * A[ 1] + B[ 1]) * base.r + base.b[ 1]) * vec4(1.0, 1.0, 1.0, base.a[ 1])),
                       add(texture2D(base.S, (t * A[ 2] + B[ 2]) * base.r + base.b[ 2]) * vec4(1.0, 1.0, 1.0, base.a[ 2]),
                           texture2D(base.S, (t * A[ 3] + B[ 3]) * base.r + base.b[ 3]) * vec4(1.0, 1.0, 1.0, base.a[ 3]))),
                   add(add(texture2D(base.S, (t * A[ 4] + B[ 4]) * base.r + base.b[ 4]) * vec4(1.0, 1.0, 1.0, base.a[ 4]),
                           texture2D(base.S, (t * A[ 5] + B[ 5]) * base.r + base.b[ 5]) * vec4(1.0, 1.0, 1.0, base.a[ 5])),
                       add(texture2D(base.S, (t * A[ 6] + B[ 6]) * base.r + base.b[ 6]) * vec4(1.0, 1.0, 1.0, base.a[ 6]),
                           texture2D(base.S, (t * A[ 7] + B[ 7]) * base.r + base.b[ 7]) * vec4(1.0, 1.0, 1.0, base.a[ 7])))),
               add(add(add(texture2D(base.S, (t * A[ 8] + B[ 8]) * base.r + base.b[ 8]) * vec4(1.0, 1.0, 1.0, base.a[ 8]),
                           texture2D(base.S, (t * A[ 9] + B[ 9]) * base.r + base.b[ 9]) * vec4(1.0, 1.0, 1.0, base.a[ 9])),
                       add(texture2D(base.S, (t * A[10] + B[10]) * base.r + base.b[10]) * vec4(1.0, 1.0, 1.0, base.a[10]),
                           texture2D(base.S, (t * A[11] + B[11]) * base.r + base.b[11]) * vec4(1.0, 1.0, 1.0, base.a[11]))),
                   add(add(texture2D(base.S, (t * A[12] + B[12]) * base.r + base.b[12]) * vec4(1.0, 1.0, 1.0, base.a[12]),
                           texture2D(base.S, (t * A[13] + B[13]) * base.r + base.b[13]) * vec4(1.0, 1.0, 1.0, base.a[13])),
                       add(texture2D(base.S, (t * A[14] + B[14]) * base.r + base.b[14]) * vec4(1.0, 1.0, 1.0, base.a[14]),
                           texture2D(base.S, (t * A[15] + B[15]) * base.r + base.b[15]) * vec4(1.0, 1.0, 1.0, base.a[15])))));
}

vec4 sample_over(vec2 t)
{
    return add(add(add(add(texture2D(over.S, (t * A[ 0] + B[ 0]) * over.r + over.b[ 0]) * vec4(1.0, 1.0, 1.0, over.a[ 0]),
                           texture2D(over.S, (t * A[ 1] + B[ 1]) * over.r + over.b[ 1]) * vec4(1.0, 1.0, 1.0, over.a[ 1])),
                       add(texture2D(over.S, (t * A[ 2] + B[ 2]) * over.r + over.b[ 2]) * vec4(1.0, 1.0, 1.0, over.a[ 2]),
                           texture2D(over.S, (t * A[ 3] + B[ 3]) * over.r + over.b[ 3]) * vec4(1.0, 1.0, 1.0, over.a[ 3]))),
                   add(add(texture2D(over.S, (t * A[ 4] + B[ 4]) * over.r + over.b[ 4]) * vec4(1.0, 1.0, 1.0, over.a[ 4]),
                           texture2D(over.S, (t * A[ 5] + B[ 5]) * over.r + over.b[ 5]) * vec4(1.0, 1.0, 1.0, over.a[ 5])),
                       add(texture2D(over.S, (t * A[ 6] + B[ 6]) * over.r + over.b[ 6]) * vec4(1.0, 1.0, 1.0, over.a[ 6]),
                           texture2D(over.S, (t * A[ 7] + B[ 7]) * over.r + over.b[ 7]) * vec4(1.0, 1.0, 1.0, over.a[ 7])))),
               add(add(add(texture2D(over.S, (t * A[ 8] + B[ 8]) * over.r + over.b[ 8]) * vec4(1.0, 1.0, 1.0, over.a[ 8]),
                           texture2D(over.S, (t * A[ 9] + B[ 9]) * over.r + over.b[ 9]) * vec4(1.0, 1.0, 1.0, over.a[ 9])),
                       add(texture2D(over.S, (t * A[10] + B[10]) * over.r + over.b[10]) * vec4(1.0, 1.0, 1.0, over.a[10]),
                           texture2D(over.S, (t * A[11] + B[11]) * over.r + over.b[11]) * vec4(1.0, 1.0, 1.0, over.a[11]))),
                   add(add(texture2D(over.S, (t * A[12] + B[12]) * over.r + over.b[12]) * vec4(1.0, 1.0, 1.0, over.a[12]),
                           texture2D(over.S, (t * A[13] + B[13]) * over.r + over.b[13]) * vec4(1.0, 1.0, 1.0, over.a[13])),
                       add(texture2D(over.S, (t * A[14] + B[14]) * over.r + over.b[14]) * vec4(1.0, 1.0, 1.0, over.a[14]),
                           texture2D(over.S, (t * A[15] + B[15]) * over.r + over.b[15]) * vec4(1.0, 1.0, 1.0, over.a[15])))));
}

//------------------------------------------------------------------------------

void main()
{
    vec3 a = mix(vec3(base.k0), vec3(base.k1), sample_base(gl_TexCoord[0].xy).rgb);
    vec3 b = mix(vec3(over.k0), vec3(over.k1), sample_over(gl_TexCoord[0].xy).rgb);
    vec3 c = step(1.0 / 255.0, b);

    gl_FragColor = vec4(mix(a, b, c), 1.0);
}
