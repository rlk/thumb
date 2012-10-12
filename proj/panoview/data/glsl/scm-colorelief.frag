#version 120

varying vec3 var_V;
varying vec3 var_L;

struct scm
{
    sampler3D img;
    vec2      siz;
    float     idx[16];
    float     age[16];
};

uniform scm normal;

uniform vec2 page_a[16];
uniform vec2 page_b[16];

//------------------------------------------------------------------------------

vec4 age(vec4 c, float k)
{
    return vec4(c.rgb, c.a * k);
}

vec4 tex(vec2 c, float k)
{
    return texture3D(normal.img, vec3((c * normal.siz + vec2(1.0)) /
                                          (normal.siz + vec2(2.0)), k));
}

vec4 img0(vec2 t)
{
    return age(tex(page_a[0] * t + page_b[0], normal.idx[0]), normal.age[0]);
}

vec4 img1(vec2 t)
{
    return age(tex(page_a[1] * t + page_b[1], normal.idx[1]), normal.age[1]);
}

vec4 img2(vec2 t)
{
    return age(tex(page_a[2] * t + page_b[2], normal.idx[2]), normal.age[2]);
}

vec4 img3(vec2 t)
{
    return age(tex(page_a[3] * t + page_b[3], normal.idx[3]), normal.age[3]);
}

vec4 img4(vec2 t)
{
    return age(tex(page_a[4] * t + page_b[4], normal.idx[4]), normal.age[4]);
}

vec4 img5(vec2 t)
{
    return age(tex(page_a[5] * t + page_b[5], normal.idx[5]), normal.age[5]);
}

vec4 img6(vec2 t)
{
    return age(tex(page_a[6] * t + page_b[6], normal.idx[6]), normal.age[6]);
}

vec4 img7(vec2 t)
{
    return age(tex(page_a[7] * t + page_b[7], normal.idx[7]), normal.age[7]);
}


vec4 blend(vec4 a, vec4 b)
{
    return vec4(mix(b.rgb, a.rgb, a.a), 1.0);
}

vec4 sample(vec2 t)
{
    vec4 c =  img0(t);
    c = blend(img1(t), c);
    c = blend(img2(t), c);
    c = blend(img3(t), c);
    c = blend(img4(t), c);
    c = blend(img5(t), c);
    c = blend(img6(t), c);
    c = blend(img7(t), c);
    return c;
}

//------------------------------------------------------------------------------

void main()
{
    vec3 V = normalize(var_V);
    vec3 L = normalize(var_L);

    vec3 N = normalize(sample(gl_TexCoord[0].xy).rgb * 2.0 - 1.0);

    float nl = max(0.0, dot(N, L));
    float nv = max(0.0, dot(N, V));
    float kd = 2.0 * nl / (nl + nv);

    gl_FragColor = vec4(gl_Color.rgb * kd, 1.0);
}
