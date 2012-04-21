
uniform float     rm;
uniform float     rb;
uniform mat3      faceM;
uniform vec2      v_mul[64];
uniform vec2      v_add[64];
uniform sampler2D v_img[64];
uniform float     v_age[64];

varying vec3 var_V;
varying vec3 var_L;

//------------------------------------------------------------------------------

vec4 tex0(vec2 t)
{
    vec4 c = texture2D(v_img[0], v_mul[0] * t + v_add[0]);
    return vec4(c.rgb, c.a * v_age[0]);
}

vec4 tex1(vec2 t)
{
    vec4 c = texture2D(v_img[1], v_mul[1] * t + v_add[1]);
    return vec4(c.rgb, c.a * v_age[1]);
}

vec4 tex2(vec2 t)
{
    vec4 c = texture2D(v_img[2], v_mul[2] * t + v_add[2]);
    return vec4(c.rgb, c.a * v_age[2]);
}

vec4 tex3(vec2 t)
{
    vec4 c = texture2D(v_img[3], v_mul[3] * t + v_add[3]);
    return vec4(c.rgb, c.a * v_age[3]);
}

vec4 tex4(vec2 t)
{
    vec4 c = texture2D(v_img[4], v_mul[4] * t + v_add[4]);
    return vec4(c.rgb, c.a * v_age[4]);
}

vec4 tex5(vec2 t)
{
    vec4 c = texture2D(v_img[5], v_mul[5] * t + v_add[5]);
    return vec4(c.rgb, c.a * v_age[5]);
}

vec4 tex6(vec2 t)
{
    vec4 c = texture2D(v_img[6], v_mul[6] * t + v_add[6]);
    return vec4(c.rgb, c.a * v_age[6]);
}

vec4 tex7(vec2 t)
{
    vec4 c = texture2D(v_img[7], v_mul[7] * t + v_add[7]);
    return vec4(c.rgb, c.a * v_age[7]);
}

//------------------------------------------------------------------------------

vec4 blend(vec4 a, vec4 b)
{
    return vec4(mix(b.rgb, a.rgb, a.a), 1.0);
}

vec4 sample(vec2 t)
{
    return blend(tex7(t),
               blend(tex6(t),
                   blend(tex5(t),
                       blend(tex4(t),
                           blend(tex3(t),
                               blend(tex2(t),
                                   blend(tex1(t), tex0(t))))))));
}

//------------------------------------------------------------------------------

float peak(float k, float c)
{
    return max(0.0, 1.0 - abs(k - c) * 5.0);
}

vec4 colormap(float k)
{
    return peak(k, 0.0) * vec4(1.0, 0.0, 1.0, 1.0) +
           peak(k, 0.2) * vec4(0.0, 0.0, 1.0, 1.0) +
           peak(k, 0.4) * vec4(0.0, 1.0, 1.0, 1.0) +
           peak(k, 0.6) * vec4(1.0, 1.0, 0.0, 1.0) +
           peak(k, 0.8) * vec4(1.0, 0.0, 0.0, 1.0) +
           peak(k, 1.0) * vec4(1.0, 1.0, 1.0, 1.0);
}

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
    float k = sample(gl_Vertex.xy).r;
    float h = k * rm + rb;
    vec3  v = h * scube(v_mul[0] * gl_Vertex.xy + v_add[0]);

    var_V = v;
    var_L = gl_LightSource[0].position.xyz;

    gl_TexCoord[0].xy = gl_Vertex.xy;
    gl_FrontColor = colormap(k);
    gl_Position = gl_ModelViewProjectionMatrix * vec4(v, 1.0);
}
