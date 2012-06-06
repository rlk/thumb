
uniform mat3 faceM;
uniform vec2 v_mul[64];
uniform vec2 v_add[64];

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
    vec3 v = scube(v_mul[0] * gl_Vertex.xy + v_add[0]);

    var_V = v;
    var_L = gl_LightSource[0].position.xyz;

    gl_TexCoord[0].xy = gl_Vertex.xy;
    gl_Position = gl_ModelViewProjectionMatrix * vec4(v, 1.0);
}
