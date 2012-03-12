varying vec4 V;

uniform vec2 tex_a[8];
uniform vec2 tex_d[8];

//uniform sampler2D image[16];
//uniform float     alpha[16];
uniform sampler2D image[8];
uniform float     alpha[8];

//------------------------------------------------------------------------------

vec4 img0(vec2 t)
{
    vec4 c = texture2D(image[0], t);
    return c;
}

vec4 img1(vec2 t)
{
    vec4 c = texture2D(image[1], (t - tex_a[1]) / (tex_d[1] - tex_a[1]));
    return vec4(c.rgb, c.a * alpha[1]);
}

vec4 img2(vec2 t)
{
    vec4 c = texture2D(image[2], (t - tex_a[2]) / (tex_d[2] - tex_a[2]));
    return vec4(c.rgb, c.a * alpha[2]);
}

vec4 img3(vec2 t)
{
    vec4 c = texture2D(image[3], (t - tex_a[3]) / (tex_d[3] - tex_a[3]));
    return vec4(c.rgb, c.a * alpha[3]);
}

vec4 img4(vec2 t)
{
    vec4 c = texture2D(image[4], (t - tex_a[4]) / (tex_d[4] - tex_a[4]));
    return vec4(c.rgb, c.a * alpha[4]);
}

vec4 img5(vec2 t)
{
    vec4 c = texture2D(image[5], (t - tex_a[5]) / (tex_d[5] - tex_a[5]));
    return vec4(c.rgb, c.a * alpha[5]);
}

vec4 img6(vec2 t)
{
    vec4 c = texture2D(image[6], (t - tex_a[6]) / (tex_d[6] - tex_a[6]));
    return vec4(c.rgb, c.a * alpha[6]);
}

vec4 img7(vec2 t)
{
    vec4 c = texture2D(image[7], (t - tex_a[7]) / (tex_d[7] - tex_a[7]));
    return vec4(c.rgb, c.a * alpha[7]);
}

//------------------------------------------------------------------------------
/*
vec4 img8(vec2 t)
{
    vec4 c = texture2D(image[8], t);
    return c;
}

vec4 img9(vec2 t)
{
    vec4 c = texture2D(image[9], (t - tex_a[1]) / (tex_d[1] - tex_a[1]));
    return vec4(c.rgb, c.a * alpha[9]);
}

vec4 img10(vec2 t)
{
    vec4 c = texture2D(image[10], (t - tex_a[2]) / (tex_d[2] - tex_a[2]));
    return vec4(c.rgb, c.a * alpha[10]);
}

vec4 img11(vec2 t)
{
    vec4 c = texture2D(image[11], (t - tex_a[3]) / (tex_d[3] - tex_a[3]));
    return vec4(c.rgb, c.a * alpha[11]);
}

vec4 img12(vec2 t)
{
    vec4 c = texture2D(image[12], (t - tex_a[4]) / (tex_d[4] - tex_a[4]));
    return vec4(c.rgb, c.a * alpha[12]);
}

vec4 img13(vec2 t)
{
    vec4 c = texture2D(image[13], (t - tex_a[5]) / (tex_d[5] - tex_a[5]));
    return vec4(c.rgb, c.a * alpha[13]);
}

vec4 img14(vec2 t)
{
    vec4 c = texture2D(image[14], (t - tex_a[6]) / (tex_d[6] - tex_a[6]));
    return vec4(c.rgb, c.a * alpha[14]);
}

vec4 img15(vec2 t)
{
    vec4 c = texture2D(image[15], (t - tex_a[7]) / (tex_d[7] - tex_a[7]));
    return vec4(c.rgb, c.a * alpha[15]);
}
*/
//------------------------------------------------------------------------------

float peak(float k, float c)
{
    return max(0.0, 1.0 - abs(k - c) * 5.0);
}

vec4 colormap(float k)
{
    return peak(k, 0.0) * vec4(1.0, 0.0, 1.0, 1.0) +
           peak(k, 0.2) * vec4(0.0, 0.0, 1.0, 1.0) +
           peak(k, 0.4) * vec4(0.0, 1.0, 1.0, 1.0) +
           peak(k, 0.6) * vec4(1.0, 1.0, 0.0, 1.0) +
           peak(k, 0.8) * vec4(1.0, 0.0, 0.0, 1.0) +
           peak(k, 1.0) * vec4(1.0, 1.0, 1.0, 1.0);
}

//------------------------------------------------------------------------------

vec4 blend(vec4 a, vec4 b)
{
    return vec4(mix(b.rgb, a.rgb, a.a), 1.0);
}

vec4 sampleA(vec2 t)
{
    return
        blend(img7(t),
            blend(img6(t),
                blend(img5(t),
                    blend(img4(t),
                        blend(img3(t),
                            blend(img2(t),
                                blend(img1(t), img0(t))))))));
}
/*
vec4 sampleB(vec2 t)
{
    return
        blend(img15(t),
            blend(img14(t),
                blend(img13(t),
                    blend(img12(t),
                        blend(img11(t),
                            blend(img10(t),
                                blend(img9(t), img8(t))))))));
}
*/
void main()
{
    vec4 D =  colormap(2.0 * sampleA(gl_TexCoord[0].xy).r);
//    vec4 D =   sampleA(gl_TexCoord[0].xy);
//    vec4 D = vec4(0.8, 0.8, 0.8, 1.0);
//    vec3 N = (sampleA(gl_TexCoord[0].xy).rgb * 2.0) - 1.0;

//    float kd = max(0.0, dot(N, gl_LightSource[0].position.xyz));
    float kd = 1.0;

    gl_FragColor = vec4(D.rgb * kd, D.a);
}

