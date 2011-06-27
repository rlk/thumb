
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

/*
vec3 mix4(vec3 a, vec3 b, vec3 c, vec3 d, vec2 k)
{
    vec3 t = mix(a, b, k.x);
    vec3 u = mix(c, d, k.x);

    return mix(t, u, k.y);
}
*/

vec3 slerp(vec3 a, vec3 b, float k)
{
    float l = 1.0 - k;
    float O = acos(dot(a, b));
    
    return a * sin(l * O) / sin(O)
         + b * sin(k * O) / sin(O);
}

vec3 mix4(vec3 a, vec3 b, vec3 c, vec3 d, vec2 k)
{
    vec3 t = slerp(a, b, k.x);
    vec3 u = slerp(c, d, k.x);

    return slerp(t, u, k.y);
}

vec2 mix4(vec2 a, vec2 b, vec2 c, vec2 d, vec2 k)
{
    vec2 t = mix(a, b, k.x);
    vec2 u = mix(c, d, k.x);

    return mix(t, u, k.y);
}

//------------------------------------------------------------------------------

void main()
{
    vec2 k = gl_Vertex.xy;

    vec2 t = mix4(tex_a[level], tex_b[level],
                  tex_c[level], tex_d[level], k);

//    vec3 v = normalize(mix4(pos_a, pos_b, pos_c, pos_d, k));
    vec3 v = normalize(mix4(pos_a, pos_b, pos_c, pos_d, t));

    gl_TexCoord[0].xy = t;

    gl_Position = gl_ModelViewProjectionMatrix * vec4(v, 1.0);
}
