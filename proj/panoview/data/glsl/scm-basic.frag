#version 120
#extension GL_EXT_texture_array : enable

struct scm
{
    sampler2DArray img;
    float          idx[16];
    float          age[16];
};

uniform scm color;

uniform vec2 page_a[16];
uniform vec2 page_b[16];

//------------------------------------------------------------------------------

vec4 age(vec4 c, float k)
{
    return vec4(c.rgb, c.a * k);
}

vec4 img0(vec2 t)
{
    vec3 c = vec3(page_a[0] * t + page_b[0], color.idx[0]);
    return age(texture2DArray(color.img, c), color.age[0]);
}

vec4 img1(vec2 t)
{
    vec3 c = vec3(page_a[1] * t + page_b[1], color.idx[1]);
    return age(texture2DArray(color.img, c), color.age[1]);
}

vec4 img2(vec2 t)
{
    vec3 c = vec3(page_a[2] * t + page_b[2], color.idx[2]);
    return age(texture2DArray(color.img, c), color.age[2]);
}

vec4 img3(vec2 t)
{
    vec3 c = vec3(page_a[3] * t + page_b[3], color.idx[3]);
    return age(texture2DArray(color.img, c), color.age[3]);
}

vec4 img4(vec2 t)
{
    vec3 c = vec3(page_a[4] * t + page_b[4], color.idx[4]);
    return age(texture2DArray(color.img, c), color.age[4]);
}

vec4 img5(vec2 t)
{
    vec3 c = vec3(page_a[5] * t + page_b[5], color.idx[5]);
    return age(texture2DArray(color.img, c), color.age[5]);
}

vec4 img6(vec2 t)
{
    vec3 c = vec3(page_a[6] * t + page_b[6], color.idx[6]);
    return age(texture2DArray(color.img, c), color.age[6]);
}

vec4 img7(vec2 t)
{
    vec3 c = vec3(page_a[7] * t + page_b[7], color.idx[7]);
    return age(texture2DArray(color.img, c), color.age[7]);
}

vec4 blend(vec4 a, vec4 b)
{
    return vec4(mix(b.rgb, a.rgb, a.a), 1.0);
}

vec4 sample(vec2 t)
{
    return blend(img7(t),
               blend(img6(t),
                   blend(img5(t),
                       blend(img4(t),
                           blend(img3(t),
                               blend(img2(t),
                                   blend(img1(t), img0(t))))))));
}

//------------------------------------------------------------------------------

void main()
{
    gl_FragColor = sample(gl_TexCoord[0].xy);
}
