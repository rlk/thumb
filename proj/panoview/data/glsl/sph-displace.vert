
uniform int level;

uniform vec2 tex_a[8];
uniform vec2 tex_d[8];

uniform mat3 face;

uniform sampler2D image[16];
uniform float     alpha[16];

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

vec4 blend(vec4 a, vec4 b)
{
    return vec4(mix(b.rgb, a.rgb, a.a), 1.0);
}

vec4 sample(vec2 t)
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

//------------------------------------------------------------------------------

vec3 scube(vec2 t)
{
    vec2  s = radians(t * 90.0 - 45.0);

    float x =  sin(s.x) * cos(s.y);
    float y = -cos(s.x) * sin(s.y);
    float z =  cos(s.x) * cos(s.y);

    return face * normalize(vec3(x, y, z));
}

//------------------------------------------------------------------------------

void main()
{
    const float r0 = 1728240.0;
    const float rm = 1737400.0;
    const float r1 = 1748170.0;

    vec2 t =  tex_a[level]
           + (tex_d[level] - tex_a[level]) * gl_Vertex.xy;

    float h = (2.0 * sample(t).r * (r1 - r0) + r0) / rm;
    vec3  v = scube(t) * h;

    gl_TexCoord[0].xy = t;

    gl_Position = gl_ModelViewProjectionMatrix * vec4(v, 1.0);
}
