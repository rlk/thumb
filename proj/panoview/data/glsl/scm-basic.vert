#version 120
#extension GL_EXT_texture_array : enable


uniform mat3 face_M;

uniform vec2 page_a[16];
uniform vec2 page_b[16];

varying vec3 var_V;
varying vec3 var_L;

//------------------------------------------------------------------------------

vec3 scube(vec2 t)
{
    vec2  s = radians(t * 90.0 - 45.0);

    float x =  sin(s.x) * cos(s.y);
    float y = -cos(s.x) * sin(s.y);
    float z =  cos(s.x) * cos(s.y);

    return face_M * normalize(vec3(x, y, z));
}

//------------------------------------------------------------------------------

void main()
{
    vec3 v = scube(page_a[0] * gl_Vertex.xy + page_b[0]);

    var_V = v;
    var_L = gl_LightSource[0].position.xyz;

    gl_TexCoord[0].xy = gl_Vertex.xy;
    gl_Position = gl_ModelViewProjectionMatrix * vec4(v, 1.0);
}
