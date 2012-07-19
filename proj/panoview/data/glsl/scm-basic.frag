#version 120

struct scm
{
    sampler3D img;
    vec2      siz;
    float     idx[16];
    float     age[16];
};

uniform scm color;

uniform vec2 page_a[16];
uniform vec2 page_b[16];

//------------------------------------------------------------------------------

vec4 age(vec4 c, float k)
{
    return vec4(c.rgb, c.a * k);
}

vec4 tex(vec2 c, float k)
{
    return texture3D(color.img, vec3((c * color.siz + vec2(1.0)) /
                                         (color.siz + vec2(2.0)), k));
}

vec4 img0(vec2 t)
{
    return age(tex(page_a[0] * t + page_b[0], color.idx[0]), color.age[0]);
}

vec4 img1(vec2 t)
{
    return age(tex(page_a[1] * t + page_b[1], color.idx[1]), color.age[1]);
}

vec4 img2(vec2 t)
{
    return age(tex(page_a[2] * t + page_b[2], color.idx[2]), color.age[2]);
}

vec4 img3(vec2 t)
{
    return age(tex(page_a[3] * t + page_b[3], color.idx[3]), color.age[3]);
}

vec4 img4(vec2 t)
{
    return age(tex(page_a[4] * t + page_b[4], color.idx[4]), color.age[4]);
}

vec4 img5(vec2 t)
{
    return age(tex(page_a[5] * t + page_b[5], color.idx[5]), color.age[5]);
}

vec4 img6(vec2 t)
{
    return age(tex(page_a[6] * t + page_b[6], color.idx[6]), color.age[6]);
}

vec4 img7(vec2 t)
{
    return age(tex(page_a[7] * t + page_b[7], color.idx[7]), color.age[7]);
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
    gl_FragColor = sample(gl_TexCoord[0].xy);
}
