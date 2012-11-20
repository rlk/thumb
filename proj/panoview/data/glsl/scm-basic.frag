#version 120

struct scm
{
    sampler2DRect img;
    vec2          A;
    vec2          B[16];
    vec4          K[16];
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
    return add(add(add(add(texture2DRect(color.img, (t * A[ 0] + B[ 0]) * color.A + color.B[ 0] + 1.0) * color.K[ 0],
                           texture2DRect(color.img, (t * A[ 1] + B[ 1]) * color.A + color.B[ 1] + 1.0) * color.K[ 1]),
                       add(texture2DRect(color.img, (t * A[ 2] + B[ 2]) * color.A + color.B[ 2] + 1.0) * color.K[ 2],
                           texture2DRect(color.img, (t * A[ 3] + B[ 3]) * color.A + color.B[ 3] + 1.0) * color.K[ 3])),
                   add(add(texture2DRect(color.img, (t * A[ 4] + B[ 4]) * color.A + color.B[ 4] + 1.0) * color.K[ 4],
                           texture2DRect(color.img, (t * A[ 5] + B[ 5]) * color.A + color.B[ 5] + 1.0) * color.K[ 5]),
                       add(texture2DRect(color.img, (t * A[ 6] + B[ 6]) * color.A + color.B[ 6] + 1.0) * color.K[ 6],
                           texture2DRect(color.img, (t * A[ 7] + B[ 7]) * color.A + color.B[ 7] + 1.0) * color.K[ 7]))),
               add(add(add(texture2DRect(color.img, (t * A[ 8] + B[ 8]) * color.A + color.B[ 8] + 1.0) * color.K[ 8],
                           texture2DRect(color.img, (t * A[ 9] + B[ 9]) * color.A + color.B[ 9] + 1.0) * color.K[ 9]),
                       add(texture2DRect(color.img, (t * A[10] + B[10]) * color.A + color.B[10] + 1.0) * color.K[10],
                           texture2DRect(color.img, (t * A[11] + B[11]) * color.A + color.B[11] + 1.0) * color.K[11])),
                   add(add(texture2DRect(color.img, (t * A[12] + B[12]) * color.A + color.B[12] + 1.0) * color.K[12],
                           texture2DRect(color.img, (t * A[13] + B[13]) * color.A + color.B[13] + 1.0) * color.K[13]),
                       add(texture2DRect(color.img, (t * A[14] + B[14]) * color.A + color.B[14] + 1.0) * color.K[14],
                           texture2DRect(color.img, (t * A[15] + B[15]) * color.A + color.B[15] + 1.0) * color.K[15]))));
}

//------------------------------------------------------------------------------

void main()
{
    gl_FragColor = sample_color(gl_TexCoord[0].xy);
}
