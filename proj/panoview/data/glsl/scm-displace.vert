#version 120
#extension GL_EXT_texture_array : enable

struct scm
{
    sampler3D img;
    float          idx[16];
    float          age[16];
};

uniform scm height;

uniform mat3 face_M;
uniform vec2 page_a[16];
uniform vec2 page_b[16];

uniform float r0;
uniform float r1;

varying vec3 var_V;
varying vec3 var_L;

//------------------------------------------------------------------------------

vec4 age(vec4 c, float k)
{
    return vec4(c.rgb, c.a * k);
}

vec3 coord(vec2 t, vec2 a, vec2 b, float z)
{
    vec2 u = a * t + b;
    return vec3((u * vec2(426.0) + vec2(1.0)) / vec2(428.0), z);
}

vec4 img0(vec2 t)
{
    vec3 c = coord(t, page_a[0], page_b[0], height.idx[0]);
    return age(texture3D(height.img, c), height.age[0]);
}

vec4 img1(vec2 t)
{
    vec3 c = coord(t, page_a[1], page_b[1], height.idx[1]);
    return age(texture3D(height.img, c), height.age[1]);
}

vec4 img2(vec2 t)
{
    vec3 c = coord(t, page_a[2], page_b[2], height.idx[2]);
    return age(texture3D(height.img, c), height.age[2]);
}

vec4 img3(vec2 t)
{
    vec3 c = coord(t, page_a[3], page_b[3], height.idx[3]);
    return age(texture3D(height.img, c), height.age[3]);
}

vec4 img4(vec2 t)
{
    vec3 c = coord(t, page_a[4], page_b[4], height.idx[4]);
    return age(texture3D(height.img, c), height.age[4]);
}

vec4 img5(vec2 t)
{
    vec3 c = coord(t, page_a[5], page_b[5], height.idx[5]);
    return age(texture3D(height.img, c), height.age[5]);
}

vec4 img6(vec2 t)
{
    vec3 c = coord(t, page_a[6], page_b[6], height.idx[6]);
    return age(texture3D(height.img, c), height.age[6]);
}

vec4 img7(vec2 t)
{
    vec3 c = coord(t, page_a[7], page_b[7], height.idx[7]);
    return age(texture3D(height.img, c), height.age[7]);
}

vec4 blend(vec4 a, vec4 b)
{
    return vec4(mix(b.rgb, a.rgb, a.a), 1.0);
}

vec4 sample(vec2 t)
{
    return blend(img7(t),
               blend(img6(t),
                   blend(img5(t),
                       blend(img4(t),
                           blend(img3(t),
                               blend(img2(t),
                                   blend(img1(t), img0(t))))))));
}

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
    float k = sample(gl_Vertex.xy).r;
    float h = mix(r0, r1, k);
    vec3  v = h * scube(page_a[0] * gl_Vertex.xy + page_b[0]);

    var_V = v;
    var_L = gl_LightSource[0].position.xyz;

    gl_TexCoord[0].xy = gl_Vertex.xy;
    gl_Position = gl_ModelViewProjectionMatrix * vec4(v, 1.0);
}
