#version 120

struct scm
{
    sampler3D img;
    vec2      siz;
    float     idx[16];
    float     age[16];
};

uniform scm base;
uniform scm over;

uniform vec2 page_a[16];
uniform vec2 page_b[16];

//------------------------------------------------------------------------------

vec4 age(vec4 c, float k)
{
    return vec4(c.rgb, c.a * k);
}

vec4 btex(vec2 c, float k)
{
    return texture3D(base.img, vec3((c * base.siz + vec2(1.0)) /
                                        (base.siz + vec2(2.0)), k));
}

vec4 otex(vec2 c, float k)
{
    return texture3D(over.img, vec3((c * over.siz + vec2(1.0)) /
                                        (over.siz + vec2(2.0)), k));
}

//------------------------------------------------------------------------------

vec4 bimg0(vec2 t)
{
    return age(btex(page_a[0] * t + page_b[0], base.idx[0]), base.age[0]);
}

vec4 bimg1(vec2 t)
{
    return age(btex(page_a[1] * t + page_b[1], base.idx[1]), base.age[1]);
}

vec4 bimg2(vec2 t)
{
    return age(btex(page_a[2] * t + page_b[2], base.idx[2]), base.age[2]);
}

vec4 bimg3(vec2 t)
{
    return age(btex(page_a[3] * t + page_b[3], base.idx[3]), base.age[3]);
}

vec4 bimg4(vec2 t)
{
    return age(btex(page_a[4] * t + page_b[4], base.idx[4]), base.age[4]);
}

vec4 bimg5(vec2 t)
{
    return age(btex(page_a[5] * t + page_b[5], base.idx[5]), base.age[5]);
}

vec4 bimg6(vec2 t)
{
    return age(btex(page_a[6] * t + page_b[6], base.idx[6]), base.age[6]);
}

vec4 bimg7(vec2 t)
{
    return age(btex(page_a[7] * t + page_b[7], base.idx[7]), base.age[7]);
}

//------------------------------------------------------------------------------

vec4 oimg0(vec2 t)
{
    return age(otex(page_a[0] * t + page_b[0], over.idx[0]), over.age[0]);
}

vec4 oimg1(vec2 t)
{
    return age(otex(page_a[1] * t + page_b[1], over.idx[1]), over.age[1]);
}

vec4 oimg2(vec2 t)
{
    return age(otex(page_a[2] * t + page_b[2], over.idx[2]), over.age[2]);
}

vec4 oimg3(vec2 t)
{
    return age(otex(page_a[3] * t + page_b[3], over.idx[3]), over.age[3]);
}

vec4 oimg4(vec2 t)
{
    return age(otex(page_a[4] * t + page_b[4], over.idx[4]), over.age[4]);
}

vec4 oimg5(vec2 t)
{
    return age(otex(page_a[5] * t + page_b[5], over.idx[5]), over.age[5]);
}

vec4 oimg6(vec2 t)
{
    return age(otex(page_a[6] * t + page_b[6], over.idx[6]), over.age[6]);
}

vec4 oimg7(vec2 t)
{
    return age(otex(page_a[7] * t + page_b[7], over.idx[7]), over.age[7]);
}

vec4 oimg8(vec2 t)
{
    return age(otex(page_a[8] * t + page_b[8], over.idx[8]), over.age[8]);
}

vec4 oimg9(vec2 t)
{
    return age(otex(page_a[9] * t + page_b[9], over.idx[9]), over.age[9]);
}

vec4 oimg10(vec2 t)
{
    return age(otex(page_a[10] * t + page_b[10], over.idx[10]), over.age[10]);
}

vec4 oimg11(vec2 t)
{
    return age(otex(page_a[11] * t + page_b[11], over.idx[11]), over.age[11]);
}

vec4 oimg12(vec2 t)
{
    return age(otex(page_a[12] * t + page_b[12], over.idx[12]), over.age[12]);
}

vec4 oimg13(vec2 t)
{
    return age(otex(page_a[13] * t + page_b[13], over.idx[13]), over.age[13]);
}

vec4 oimg14(vec2 t)
{
    return age(otex(page_a[14] * t + page_b[14], over.idx[14]), over.age[14]);
}

vec4 oimg15(vec2 t)
{
    return age(otex(page_a[15] * t + page_b[15], over.idx[15]), over.age[15]);
}

//------------------------------------------------------------------------------

vec4 blend(vec4 a, vec4 b)
{
    return vec4(mix(b.rgb, a.rgb, a.a), 1.0);
}

vec4 bsample(vec2 t)
{
    vec4 c =  bimg0(t);
    c = blend(bimg1(t), c);
    c = blend(bimg2(t), c);
    c = blend(bimg3(t), c);
    c = blend(bimg4(t), c);
    c = blend(bimg5(t), c);
    c = blend(bimg6(t), c);
    c = blend(bimg7(t), c);
    return c;
}

vec4 osample(vec2 t)
{
    vec4 c =  oimg0(t);
    c = blend(oimg1(t), c);
    c = blend(oimg2(t), c);
    c = blend(oimg3(t), c);
    c = blend(oimg4(t), c);
    c = blend(oimg5(t), c);
    c = blend(oimg6(t), c);
    c = blend(oimg7(t), c);
    c = blend(oimg8(t), c);
    c = blend(oimg9(t), c);
    c = blend(oimg10(t), c);
    c = blend(oimg11(t), c);
    c = blend(oimg12(t), c);
    c = blend(oimg13(t), c);
    c = blend(oimg14(t), c);
    c = blend(oimg15(t), c);
    return c;
}

//------------------------------------------------------------------------------

void main()
{
    vec3 a = bsample(gl_TexCoord[0].xy).rgb;
    vec3 b = osample(gl_TexCoord[0].xy).rgb;
    vec3 c;

    if (b.r > 0.0)
        c = b;
    else
        c = a;

    gl_FragColor = vec4(c, 1.0);
}
