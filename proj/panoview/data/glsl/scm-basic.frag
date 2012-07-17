#version 120
#extension GL_EXT_texture_array : enable

struct scm
{
    sampler2DArray img;
    int            idx[16];
    vec2           mul[16];
    vec2           add[16];
    float          age[16];
};

uniform scm color;

//------------------------------------------------------------------------------

vec4 img0(const scm s, vec2 t)
{
//  vec3 c = vec3(t * s.mul[0] + s.add[0], s.idx[0]);
    vec3 c = vec3(t, 0.0);
    vec4 p = texture2DArray(s.img, c);
    return p;
//  return vec4(p.rgb, p.a * s.age[0]);
}

//------------------------------------------------------------------------------

//vec4 img0(vec2 t)
//{
//    vec4 c = texture2D(f_img[0], f_mul[0] * t + f_add[0]);
//    return vec4(c.rgb, c.a * f_age[0]);
//}
//
//vec4 img1(vec2 t)
//{
//    vec4 c = texture2D(f_img[1], f_mul[1] * t + f_add[1]);
//    return vec4(c.rgb, c.a * f_age[1]);
//}
//
//vec4 img2(vec2 t)
//{
//    vec4 c = texture2D(f_img[2], f_mul[2] * t + f_add[2]);
//    return vec4(c.rgb, c.a * f_age[2]);
//}
//
//vec4 img3(vec2 t)
//{
//    vec4 c = texture2D(f_img[3], f_mul[3] * t + f_add[3]);
//    return vec4(c.rgb, c.a * f_age[3]);
//}
//
//vec4 img4(vec2 t)
//{
//    vec4 c = texture2D(f_img[4], f_mul[4] * t + f_add[4]);
//    return vec4(c.rgb, c.a * f_age[4]);
//}
//
//vec4 img5(vec2 t)
//{
//    vec4 c = texture2D(f_img[5], f_mul[5] * t + f_add[5]);
//    return vec4(c.rgb, c.a * f_age[5]);
//}
//
//vec4 img6(vec2 t)
//{
//    vec4 c = texture2D(f_img[6], f_mul[6] * t + f_add[6]);
//    return vec4(c.rgb, c.a * f_age[6]);
//}
//
//vec4 img7(vec2 t)
//{
//    vec4 c = texture2D(f_img[7], f_mul[7] * t + f_add[7]);
//    return vec4(c.rgb, c.a * f_age[7]);
//}
//
//------------------------------------------------------------------------------

//vec4 blend(vec4 a, vec4 b)
//{
//    return vec4(mix(b.rgb, a.rgb, a.a), 1.0);
//}
//
//vec4 sample(vec2 t)
//{
//    return blend(img7(t),
//               blend(img6(t),
//                   blend(img5(t),
//                       blend(img4(t),
//                           blend(img3(t),
//                               blend(img2(t),
//                                   blend(img1(t), img0(t))))))));
//}
//
//------------------------------------------------------------------------------

void main()
{
//    gl_FragColor = sample(gl_TexCoord[0].xy);
    gl_FragColor = img0(color, gl_TexCoord[0].xy);
}
