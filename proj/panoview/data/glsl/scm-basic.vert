#version 120
#extension GL_EXT_texture_array : enable

struct scm
{
    sampler2DArray img;
    int            idx[16];
    vec2           mul[16];
    vec2           add[16];
    float          age[16];
};

uniform scm  color;
uniform mat3 faceM;
varying vec3 var_V;
varying vec3 var_L;


//------------------------------------------------------------------------------

vec3 scube(vec2 t)
{
    vec2  s = radians(t * 90.0 - 45.0);

    float x =  sin(s.x) * cos(s.y);
    float y = -cos(s.x) * sin(s.y);
    float z =  cos(s.x) * cos(s.y);

    return faceM * normalize(vec3(x, y, z));
}

//------------------------------------------------------------------------------

void main()
{
    vec3 v = scube(color.mul[0] * gl_Vertex.xy + color.add[0]);

    var_V = v;
    var_L = gl_LightSource[0].position.xyz;

    gl_TexCoord[0].xy = gl_Vertex.xy;
    gl_Position = gl_ModelViewProjectionMatrix * vec4(v, 1.0);
}
