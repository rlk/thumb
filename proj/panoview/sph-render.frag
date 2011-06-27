
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
/*
void main()
{   
    if      (level == 3) gl_FragColor = texture2D(texture[3], gl_TexCoord[3].xy);
    else if (level == 2) gl_FragColor = texture2D(texture[2], gl_TexCoord[2].xy);
    else if (level == 1) gl_FragColor = texture2D(texture[1], gl_TexCoord[1].xy);
    else                 gl_FragColor = texture2D(texture[0], gl_TexCoord[0].xy);
}
*/
/*
vec4 sample(float k)
{
    int   i = int(floor(k));
    float t =     fract(k);
    
    if      (i == 0) return mix(texture2D(texture[0], gl_TexCoord[0].xy),
                                texture2D(texture[1], gl_TexCoord[1].xy), t);
    else if (i == 1) return mix(texture2D(texture[1], gl_TexCoord[1].xy),
                                texture2D(texture[2], gl_TexCoord[2].xy), t);
    else if (i == 2) return mix(texture2D(texture[2], gl_TexCoord[2].xy),
                                texture2D(texture[3], gl_TexCoord[3].xy), t);
    else if (i == 3) return mix(texture2D(texture[3], gl_TexCoord[3].xy),
                                texture2D(texture[4], gl_TexCoord[4].xy), t);
    else             return     texture2D(texture[4], gl_TexCoord[4].xy);
                
}
*/

/*
    gl_TexCoord[1].xy = tex(tex_a[1], tex_b[1], tex_c[1], tex_d[1], t);
    gl_TexCoord[2].xy = tex(tex_a[2], tex_b[2], tex_c[2], tex_d[2], t);
    gl_TexCoord[3].xy = tex(tex_a[3], tex_b[3], tex_c[3], tex_d[3], t);
    gl_TexCoord[4].xy = tex(tex_a[4], tex_b[4], tex_c[4], tex_d[4], t);
    gl_TexCoord[5].xy = tex(tex_a[5], tex_b[5], tex_c[5], tex_d[5], t);
    gl_TexCoord[6].xy = tex(tex_a[6], tex_b[6], tex_c[6], tex_d[6], t);
    gl_TexCoord[7].xy = tex(tex_a[7], tex_b[7], tex_c[7], tex_d[7], t);
*/

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

