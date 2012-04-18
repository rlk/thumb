
uniform vec2      tex_a[64];
uniform vec2      tex_d[64];
uniform sampler2D f_img[64];
uniform float     f_age[64];

//------------------------------------------------------------------------------

vec4 img0(vec2 t)
{
    vec4 c = texture2D(f_img[0], t);
    return c;
}

vec4 img1(vec2 t)
{
    vec4 c = texture2D(f_img[1], (t - tex_a[1]) / (tex_d[1] - tex_a[1]));
    return vec4(c.rgb, c.a * f_age[1]);
}

vec4 img2(vec2 t)
{
    vec4 c = texture2D(f_img[2], (t - tex_a[2]) / (tex_d[2] - tex_a[2]));
    return vec4(c.rgb, c.a * f_age[2]);
}

vec4 img3(vec2 t)
{
    vec4 c = texture2D(f_img[3], (t - tex_a[3]) / (tex_d[3] - tex_a[3]));
    return vec4(c.rgb, c.a * f_age[3]);
}

vec4 img4(vec2 t)
{
    vec4 c = texture2D(f_img[4], (t - tex_a[4]) / (tex_d[4] - tex_a[4]));
    return vec4(c.rgb, c.a * f_age[4]);
}

vec4 img5(vec2 t)
{
    vec4 c = texture2D(f_img[5], (t - tex_a[5]) / (tex_d[5] - tex_a[5]));
    return vec4(c.rgb, c.a * f_age[5]);
}

vec4 img6(vec2 t)
{
    vec4 c = texture2D(f_img[6], (t - tex_a[6]) / (tex_d[6] - tex_a[6]));
    return vec4(c.rgb, c.a * f_age[6]);
}

vec4 img7(vec2 t)
{
    vec4 c = texture2D(f_img[7], (t - tex_a[7]) / (tex_d[7] - tex_a[7]));
    return vec4(c.rgb, c.a * f_age[7]);
}

vec4 img8(vec2 t)
{
    vec4 c = texture2D(f_img[8], (t - tex_a[8]) / (tex_d[8] - tex_a[8]));
    return vec4(c.rgb, c.a * f_age[8]);
}

vec4 img9(vec2 t)
{
    vec4 c = texture2D(f_img[9], (t - tex_a[9]) / (tex_d[9] - tex_a[9]));
    return vec4(c.rgb, c.a * f_age[9]);
}

vec4 img10(vec2 t)
{
    vec4 c = texture2D(f_img[10], (t - tex_a[10]) / (tex_d[10] - tex_a[10]));
    return vec4(c.rgb, c.a * f_age[10]);
}

vec4 img11(vec2 t)
{
    vec4 c = texture2D(f_img[11], (t - tex_a[11]) / (tex_d[11] - tex_a[11]));
    return vec4(c.rgb, c.a * f_age[11]);
}

vec4 img12(vec2 t)
{
    vec4 c = texture2D(f_img[12], (t - tex_a[12]) / (tex_d[12] - tex_a[12]));
    return vec4(c.rgb, c.a * f_age[12]);
}

vec4 img13(vec2 t)
{
    vec4 c = texture2D(f_img[13], (t - tex_a[13]) / (tex_d[13] - tex_a[13]));
    return vec4(c.rgb, c.a * f_age[13]);
}

vec4 img14(vec2 t)
{
    vec4 c = texture2D(f_img[14], (t - tex_a[14]) / (tex_d[14] - tex_a[14]));
    return vec4(c.rgb, c.a * f_age[14]);
}

vec4 img15(vec2 t)
{
    vec4 c = texture2D(f_img[15], (t - tex_a[15]) / (tex_d[15] - tex_a[15]));
    return vec4(c.rgb, c.a * f_age[15]);
}

//------------------------------------------------------------------------------

vec4 blend(vec4 a, vec4 b)
{
    return vec4(mix(b.rgb, a.rgb, a.a), 1.0);
}

vec4 sample(vec2 t)
{
    vec4 c =  img0 (t);
    c = blend(img1 (t), c);
    c = blend(img2 (t), c);
    c = blend(img3 (t), c);
    c = blend(img4 (t), c);
    c = blend(img5 (t), c);
    c = blend(img6 (t), c);
    c = blend(img7 (t), c);
    c = blend(img8 (t), c);
    c = blend(img9 (t), c);
    c = blend(img10(t), c);
    c = blend(img11(t), c);
    c = blend(img12(t), c);
    c = blend(img13(t), c);
    c = blend(img14(t), c);
    c = blend(img15(t), c);
    return c;
}

//------------------------------------------------------------------------------

void main()
{
    gl_FragColor = sample(gl_TexCoord[0].xy);
}
