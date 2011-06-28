
uniform float size;
uniform int   level;
uniform int   depth;

uniform vec2 tex_a[8];
uniform vec2 tex_d[8];

uniform sampler2D texture[8];

//------------------------------------------------------------------------------

float sat(float k)
{
    return clamp(k, 0.0, 1.0);
}

vec3 color(float k)
{
    return vec3(sat(k - 4.0) + sat(2.0 - k),
                sat(k      ) * sat(4.0 - k),
                sat(k - 2.0) * sat(6.0 - k));
}

//------------------------------------------------------------------------------

vec2 tex(vec2 a, vec2 d, vec2 t)
{
    return (t - a) / (d - a);
}

vec4 sample(float k)
{
    vec2 u = gl_TexCoord[0].xy;

    int   i = int(floor(k));
    float t =     fract(k);
    
    if      (i == 0) return mix(texture2D(texture[0], u),
                                texture2D(texture[1], tex(tex_a[1], tex_d[1], u)), t);
    else if (i == 1) return mix(texture2D(texture[1], tex(tex_a[1], tex_d[1], u)),
                                texture2D(texture[2], tex(tex_a[2], tex_d[2], u)), t);
    else if (i == 2) return mix(texture2D(texture[2], tex(tex_a[2], tex_d[2], u)),
                                texture2D(texture[3], tex(tex_a[3], tex_d[3], u)), t);
    else if (i == 3) return mix(texture2D(texture[3], tex(tex_a[3], tex_d[3], u)),
                                texture2D(texture[4], tex(tex_a[4], tex_d[4], u)), t);
    else             return     texture2D(texture[4], tex(tex_a[4], tex_d[4], u));
}

void main()
{
    vec2  w = fwidth(gl_TexCoord[0].xy * size);
    float r = max(w.x, w.y);
    float k = clamp(-log2(r), 0.0, float(level));
    
    vec3 c = sample(k).rgb;

    gl_FragColor = vec4(c, 1.0) * gl_FrontMaterial.diffuse;
}

