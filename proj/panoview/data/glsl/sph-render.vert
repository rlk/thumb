
uniform int level;

uniform vec2 tex_a[8];
uniform vec2 tex_d[8];

uniform mat3 face;

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
    vec2 t =  tex_a[level]
           + (tex_d[level] - tex_a[level]) * gl_Vertex.xy;

    vec3 v = scube(t);

    gl_TexCoord[0].xy = t;

    gl_Position = gl_ModelViewProjectionMatrix * vec4(v, 1.0);
}
