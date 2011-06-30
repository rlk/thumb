
uniform vec2  tex_a[8];
uniform vec2  tex_d[8];
uniform float tex_t[8];

uniform sampler2D texture[8];

//------------------------------------------------------------------------------

// Some hardware disallows accessing a sampler array element using a computed
// index. So, we need to unroll these.

vec4 tex0(vec2 t)
{
    vec4 c = texture2D(texture[0], t);
    return c;
}

vec4 tex1(vec2 t)
{
    vec4 c = texture2D(texture[1], (t - tex_a[1]) / (tex_d[1] - tex_a[1]));
    c.a *= tex_t[1];
    return c;
}

vec4 tex2(vec2 t)
{
    vec4 c = texture2D(texture[2], (t - tex_a[2]) / (tex_d[2] - tex_a[2]));
    c.a *= tex_t[2];
    return c;
}

vec4 tex3(vec2 t)
{
    vec4 c = texture2D(texture[3], (t - tex_a[3]) / (tex_d[3] - tex_a[3]));
    c.a *= tex_t[3];
    return c;
}

vec4 tex4(vec2 t)
{
    vec4 c = texture2D(texture[4], (t - tex_a[4]) / (tex_d[4] - tex_a[4]));
    c.a *= tex_t[4];
    return c;
}

vec4 tex5(vec2 t)
{
    vec4 c = texture2D(texture[5], (t - tex_a[5]) / (tex_d[5] - tex_a[5]));
    c.a *= tex_t[5];
    return c;
}

vec4 tex6(vec2 t)
{
    vec4 c = texture2D(texture[6], (t - tex_a[6]) / (tex_d[6] - tex_a[6]));
    c.a *= tex_t[6];
    return c;
}

vec4 tex7(vec2 t)
{
    vec4 c = texture2D(texture[7], (t - tex_a[7]) / (tex_d[7] - tex_a[7]));
    c.a *= tex_t[7];
    return c;
}

//------------------------------------------------------------------------------

vec4 blend(vec4 a, vec4 b)
{
    return vec4(mix(b.rgb, a.rgb, a.a), 1.0);
}

vec4 sample(vec2 t)
{
    return
        blend(tex7(t),
            blend(tex6(t),
                blend(tex5(t),
                    blend(tex4(t),
                        blend(tex3(t),
                            blend(tex2(t),
                                blend(tex1(t), tex0(t))))))));
}

void main()
{
    gl_FragColor = sample(gl_TexCoord[0].xy);
}

