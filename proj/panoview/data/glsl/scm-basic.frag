
uniform vec2      f_mul[64];
uniform vec2      f_add[64];
uniform sampler2D f_img[64];
uniform float     f_age[64];

//------------------------------------------------------------------------------

vec4 img0(vec2 t)
{
    vec4 c = texture2D(f_img[0], f_mul[0] * t + f_add[0]);
    return vec4(c.rgb, c.a * f_age[0]);
}

vec4 img1(vec2 t)
{
    vec4 c = texture2D(f_img[1], f_mul[1] * t + f_add[1]);
    return vec4(c.rgb, c.a * f_age[1]);
}

vec4 img2(vec2 t)
{
    vec4 c = texture2D(f_img[2], f_mul[2] * t + f_add[2]);
    return vec4(c.rgb, c.a * f_age[2]);
}

vec4 img3(vec2 t)
{
    vec4 c = texture2D(f_img[3], f_mul[3] * t + f_add[3]);
    return vec4(c.rgb, c.a * f_age[3]);
}

vec4 img4(vec2 t)
{
    vec4 c = texture2D(f_img[4], f_mul[4] * t + f_add[4]);
    return vec4(c.rgb, c.a * f_age[4]);
}

vec4 img5(vec2 t)
{
    vec4 c = texture2D(f_img[5], f_mul[5] * t + f_add[5]);
    return vec4(c.rgb, c.a * f_age[5]);
}

vec4 img6(vec2 t)
{
    vec4 c = texture2D(f_img[6], f_mul[6] * t + f_add[6]);
    return vec4(c.rgb, c.a * f_age[6]);
}

vec4 img7(vec2 t)
{
    vec4 c = texture2D(f_img[7], f_mul[7] * t + f_add[7]);
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
