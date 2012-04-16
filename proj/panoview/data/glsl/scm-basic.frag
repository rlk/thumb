
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

//------------------------------------------------------------------------------

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
//  gl_FragColor = gl_Color;
    gl_FragColor = sample(gl_TexCoord[0].xy);
}
