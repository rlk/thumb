
uniform int  level;

uniform vec3 pos_a;
uniform vec3 pos_b;
uniform vec3 pos_c;
uniform vec3 pos_d;

uniform vec2 tex_a[8];
uniform vec2 tex_b[8];
uniform vec2 tex_c[8];
uniform vec2 tex_d[8];

//------------------------------------------------------------------------------

vec3 mix4(vec3 a, vec3 b, vec3 c, vec3 d, vec2 k)
{
    vec3 t = mix(a, b, k.x);
    vec3 u = mix(c, d, k.x);

    return mix(t, u, k.y);
}

vec2 mix4(vec2 a, vec2 b, vec2 c, vec2 d, vec2 k)
{
    vec2 t = mix(a, b, k.x);
    vec2 u = mix(c, d, k.x);

    return mix(t, u, k.y);
}

vec2 tex(vec2 a, vec2 b, vec2 c, vec2 d, vec2 t)
{
    return (t - a) / (d - a);
}

//------------------------------------------------------------------------------

void main()
{
    vec2 k = gl_Vertex.xy;

    // Compute the position vector.

    vec3 v = normalize(mix4(pos_a, pos_b, pos_c, pos_d, k));

    // Compute the root texture coordinate from that of the current level.

    gl_TexCoord[0].xy = mix4(tex_a[level], tex_b[level],
                             tex_c[level], tex_d[level], k);
/*
    vec2 t = mix4(tex_a[level], tex_b[level],
                  tex_c[level], tex_d[level], k);

    gl_TexCoord[0].xy = tex(tex_a[0], tex_b[0], tex_c[0], tex_d[0], t);
    gl_TexCoord[1].xy = tex(tex_a[1], tex_b[1], tex_c[1], tex_d[1], t);
    gl_TexCoord[2].xy = tex(tex_a[2], tex_b[2], tex_c[2], tex_d[2], t);
    gl_TexCoord[3].xy = tex(tex_a[3], tex_b[3], tex_c[3], tex_d[3], t);
    gl_TexCoord[4].xy = tex(tex_a[4], tex_b[4], tex_c[4], tex_d[4], t);
    gl_TexCoord[5].xy = tex(tex_a[5], tex_b[5], tex_c[5], tex_d[5], t);
    gl_TexCoord[6].xy = tex(tex_a[6], tex_b[6], tex_c[6], tex_d[6], t);
    gl_TexCoord[7].xy = tex(tex_a[7], tex_b[7], tex_c[7], tex_d[7], t);
*/    
    // TODO: Benchmark these to see if cost is associated with count.

    gl_Position = gl_ModelViewProjectionMatrix * vec4(v, 1.0);
}
