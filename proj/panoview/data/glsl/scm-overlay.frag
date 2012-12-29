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

vec4 overlay(vec4 a, float aa, vec4 b, float bb)
{
    vec4 A = vec4(a.rgb, aa);
    vec4 B = vec4(b.rgb, bb);

    return mix(A, add(A, B), step(1.0, b.a));
}

vec4 sample0(vec2 t)
{
    return overlay(texture2D(base.S, (t * A[ 0] + B[ 0]) * base.r + base.b[ 0]), base.a[ 0],
                   texture2D(over.S, (t * A[ 0] + B[ 0]) * over.r + over.b[ 0]), over.a[ 0]);
}

vec4 sample1(vec2 t)
{
    return overlay(texture2D(base.S, (t * A[ 1] + B[ 1]) * base.r + base.b[ 1]), base.a[ 1],
                   texture2D(over.S, (t * A[ 1] + B[ 1]) * over.r + over.b[ 1]), over.a[ 1]);
}

vec4 sample2(vec2 t)
{
    return overlay(texture2D(base.S, (t * A[ 2] + B[ 2]) * base.r + base.b[ 2]), base.a[ 2],
                   texture2D(over.S, (t * A[ 2] + B[ 2]) * over.r + over.b[ 2]), over.a[ 2]);
}

vec4 sample3(vec2 t)
{
    return overlay(texture2D(base.S, (t * A[ 3] + B[ 3]) * base.r + base.b[ 3]), base.a[ 3],
                   texture2D(over.S, (t * A[ 3] + B[ 3]) * over.r + over.b[ 3]), over.a[ 3]);
}

vec4 sample4(vec2 t)
{
    return overlay(texture2D(base.S, (t * A[ 4] + B[ 4]) * base.r + base.b[ 4]), base.a[ 4],
                   texture2D(over.S, (t * A[ 4] + B[ 4]) * over.r + over.b[ 4]), over.a[ 4]);
}

vec4 sample5(vec2 t)
{
    return overlay(texture2D(base.S, (t * A[ 5] + B[ 5]) * base.r + base.b[ 5]), base.a[ 5],
                   texture2D(over.S, (t * A[ 5] + B[ 5]) * over.r + over.b[ 5]), over.a[ 5]);
}

vec4 sample6(vec2 t)
{
    return overlay(texture2D(base.S, (t * A[ 6] + B[ 6]) * base.r + base.b[ 6]), base.a[ 6],
                   texture2D(over.S, (t * A[ 6] + B[ 6]) * over.r + over.b[ 6]), over.a[ 6]);
}

vec4 sample7(vec2 t)
{
    return overlay(texture2D(base.S, (t * A[ 7] + B[ 7]) * base.r + base.b[ 7]), base.a[ 7],
                   texture2D(over.S, (t * A[ 7] + B[ 7]) * over.r + over.b[ 7]), over.a[ 7]);
}

vec4 sample8(vec2 t)
{
    return overlay(texture2D(base.S, (t * A[ 8] + B[ 8]) * base.r + base.b[ 8]), base.a[ 8],
                   texture2D(over.S, (t * A[ 8] + B[ 8]) * over.r + over.b[ 8]), over.a[ 8]);
}

vec4 sample9(vec2 t)
{
    return overlay(texture2D(base.S, (t * A[ 9] + B[ 9]) * base.r + base.b[ 9]), base.a[ 9],
                   texture2D(over.S, (t * A[ 9] + B[ 9]) * over.r + over.b[ 9]), over.a[ 9]);
}

vec4 sampleA(vec2 t)
{
    return overlay(texture2D(base.S, (t * A[10] + B[10]) * base.r + base.b[10]), base.a[10],
                   texture2D(over.S, (t * A[10] + B[10]) * over.r + over.b[10]), over.a[10]);
}

vec4 sampleB(vec2 t)
{
    return overlay(texture2D(base.S, (t * A[11] + B[11]) * base.r + base.b[11]), base.a[11],
                   texture2D(over.S, (t * A[11] + B[11]) * over.r + over.b[11]), over.a[11]);
}

vec4 sampleC(vec2 t)
{
    return overlay(texture2D(base.S, (t * A[12] + B[12]) * base.r + base.b[12]), base.a[12],
                   texture2D(over.S, (t * A[12] + B[12]) * over.r + over.b[12]), over.a[12]);
}

vec4 sampleD(vec2 t)
{
    return overlay(texture2D(base.S, (t * A[13] + B[13]) * base.r + base.b[13]), base.a[13],
                   texture2D(over.S, (t * A[13] + B[13]) * over.r + over.b[13]), over.a[13]);
}

vec4 sampleE(vec2 t)
{
    return overlay(texture2D(base.S, (t * A[14] + B[14]) * base.r + base.b[14]), base.a[14],
                   texture2D(over.S, (t * A[14] + B[14]) * over.r + over.b[14]), over.a[14]);
}

vec4 sampleF(vec2 t)
{
    return overlay(texture2D(base.S, (t * A[15] + B[15]) * base.r + base.b[15]), base.a[15],
                   texture2D(over.S, (t * A[15] + B[15]) * over.r + over.b[15]), over.a[15]);
}

vec4 sample(vec2 t)
{
    vec4 c   = sample0(t);
    c = add(c, sample2(t));
    c = add(c, sample3(t));
    c = add(c, sample4(t));
    c = add(c, sample5(t));
    c = add(c, sample6(t));
    c = add(c, sample7(t));
    c = add(c, sample8(t));
    c = add(c, sample9(t));
    c = add(c, sampleA(t));
    c = add(c, sampleB(t));
    c = add(c, sampleC(t));
    c = add(c, sampleD(t));
    c = add(c, sampleE(t));
    c = add(c, sampleF(t));
    return c;
}

//------------------------------------------------------------------------------

void main()
{
    gl_FragColor = vec4(sample(gl_TexCoord[0].xy).rgb, 1.0);
}
