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

uniform scm color;
uniform scm normal;

uniform vec2 page_a[16];
uniform vec2 page_b[16];

//------------------------------------------------------------------------------

vec4 age(vec4 c, float k)
{
    return vec4(c.rgb, c.a * k);
}

vec4 btex(vec2 c, float k)
{
    return texture3D(color.img, vec3((c * color.siz + vec2(1.0)) /
                                         (color.siz + vec2(2.0)), k));
}

vec4 otex(vec2 c, float k)
{
    return texture3D(normal.img, vec3((c * normal.siz + vec2(1.0)) /
                                          (normal.siz + vec2(2.0)), k));
}

//------------------------------------------------------------------------------

vec4 cimg0(vec2 t)
{
    return age(btex(page_a[0] * t + page_b[0], color.idx[0]), color.age[0]);
}

vec4 cimg1(vec2 t)
{
    return age(btex(page_a[1] * t + page_b[1], color.idx[1]), color.age[1]);
}

vec4 cimg2(vec2 t)
{
    return age(btex(page_a[2] * t + page_b[2], color.idx[2]), color.age[2]);
}

vec4 cimg3(vec2 t)
{
    return age(btex(page_a[3] * t + page_b[3], color.idx[3]), color.age[3]);
}

vec4 cimg4(vec2 t)
{
    return age(btex(page_a[4] * t + page_b[4], color.idx[4]), color.age[4]);
}

vec4 cimg5(vec2 t)
{
    return age(btex(page_a[5] * t + page_b[5], color.idx[5]), color.age[5]);
}

vec4 cimg6(vec2 t)
{
    return age(btex(page_a[6] * t + page_b[6], color.idx[6]), color.age[6]);
}

vec4 cimg7(vec2 t)
{
    return age(btex(page_a[7] * t + page_b[7], color.idx[7]), color.age[7]);
}

//------------------------------------------------------------------------------

vec4 nimg0(vec2 t)
{
    return age(otex(page_a[0] * t + page_b[0], normal.idx[0]), normal.age[0]);
}

vec4 nimg1(vec2 t)
{
    return age(otex(page_a[1] * t + page_b[1], normal.idx[1]), normal.age[1]);
}

vec4 nimg2(vec2 t)
{
    return age(otex(page_a[2] * t + page_b[2], normal.idx[2]), normal.age[2]);
}

vec4 nimg3(vec2 t)
{
    return age(otex(page_a[3] * t + page_b[3], normal.idx[3]), normal.age[3]);
}

vec4 nimg4(vec2 t)
{
    return age(otex(page_a[4] * t + page_b[4], normal.idx[4]), normal.age[4]);
}

vec4 nimg5(vec2 t)
{
    return age(otex(page_a[5] * t + page_b[5], normal.idx[5]), normal.age[5]);
}

vec4 nimg6(vec2 t)
{
    return age(otex(page_a[6] * t + page_b[6], normal.idx[6]), normal.age[6]);
}

vec4 nimg7(vec2 t)
{
    return age(otex(page_a[7] * t + page_b[7], normal.idx[7]), normal.age[7]);
}

vec4 nimg8(vec2 t)
{
    return age(otex(page_a[8] * t + page_b[8], normal.idx[8]), normal.age[8]);
}

vec4 nimg9(vec2 t)
{
    return age(otex(page_a[9] * t + page_b[9], normal.idx[9]), normal.age[9]);
}

vec4 nimg10(vec2 t)
{
    return age(otex(page_a[10] * t + page_b[10], normal.idx[10]), normal.age[10]);
}

vec4 nimg11(vec2 t)
{
    return age(otex(page_a[11] * t + page_b[11], normal.idx[11]), normal.age[11]);
}

vec4 nimg12(vec2 t)
{
    return age(otex(page_a[12] * t + page_b[12], normal.idx[12]), normal.age[12]);
}

vec4 nimg13(vec2 t)
{
    return age(otex(page_a[13] * t + page_b[13], normal.idx[13]), normal.age[13]);
}

vec4 nimg14(vec2 t)
{
    return age(otex(page_a[14] * t + page_b[14], normal.idx[14]), normal.age[14]);
}

vec4 nimg15(vec2 t)
{
    return age(otex(page_a[15] * t + page_b[15], normal.idx[15]), normal.age[15]);
}

//------------------------------------------------------------------------------

vec4 blend(vec4 a, vec4 b)
{
    return vec4(mix(b.rgb, a.rgb, a.a), 1.0);
}

vec4 csample(vec2 t)
{
    vec4 c =  cimg0(t);
    c = blend(cimg1(t), c);
    c = blend(cimg2(t), c);
    c = blend(cimg3(t), c);
    c = blend(cimg4(t), c);
    c = blend(cimg5(t), c);
    c = blend(cimg6(t), c);
    c = blend(cimg7(t), c);
    return c;
}

vec4 nsample(vec2 t)
{
    vec4 c =  nimg0(t);
    c = blend(nimg1(t), c);
    c = blend(nimg2(t), c);
    c = blend(nimg3(t), c);
    c = blend(nimg4(t), c);
    c = blend(nimg5(t), c);
    c = blend(nimg6(t), c);
    c = blend(nimg7(t), c);
    c = blend(nimg8(t), c);
    c = blend(nimg9(t), c);
    c = blend(nimg10(t), c);
    c = blend(nimg11(t), c);
    c = blend(nimg12(t), c);
    c = blend(nimg13(t), c);
    c = blend(nimg14(t), c);
    c = blend(nimg15(t), c);
    return c;
}

//------------------------------------------------------------------------------

void main()
{
    vec3 V = normalize(var_V);
    vec3 L = normalize(var_L);

    vec3 D = vec3(1.0);//          csample(gl_TexCoord[0].xy).rgb;
    vec3 N = normalize(nsample(gl_TexCoord[0].xy).rgb * 2.0 - 1.0);

    float nl = max(0.0, dot(N, L));
    float nv = max(0.0, dot(N, V));
    float kd = 2.0 * nl / (nl + nv);

    gl_FragColor = vec4(D * kd, 1.0);
}

