
uniform float size;
uniform int   level;
uniform int   depth;

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

void main()
{
    vec2  w = fwidth(gl_TexCoord[0].xy * size);
    float r = max(w.x, w.y);
    float l = log2(r);
    float k = clamp(-l, 0.0, float(depth));
    
//    vec3 c = color(k);
    vec3 c = sample(k).rgb;

    gl_FragColor = vec4(c, 1.0) * gl_FrontMaterial.diffuse;
}

