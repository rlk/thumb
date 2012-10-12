#version 120

struct scm
{
    sampler3D img;
    vec2      siz;
    float     idx[16];
    float     age[16];
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

vec4 tex(vec2 c, float k)
{
    return texture3D(height.img, vec3((c * height.siz + vec2(1.0)) /
                                          (height.siz + vec2(2.0)), k));
}

vec4 img0(vec2 t)
{
    return age(tex(page_a[0] * t + page_b[0], height.idx[0]), height.age[0]);
}

vec4 img1(vec2 t)
{
    return age(tex(page_a[1] * t + page_b[1], height.idx[1]), height.age[1]);
}

vec4 img2(vec2 t)
{
    return age(tex(page_a[2] * t + page_b[2], height.idx[2]), height.age[2]);
}

vec4 img3(vec2 t)
{
    return age(tex(page_a[3] * t + page_b[3], height.idx[3]), height.age[3]);
}

vec4 img4(vec2 t)
{
    return age(tex(page_a[4] * t + page_b[4], height.idx[4]), height.age[4]);
}

vec4 img5(vec2 t)
{
    return age(tex(page_a[5] * t + page_b[5], height.idx[5]), height.age[5]);
}

vec4 img6(vec2 t)
{
    return age(tex(page_a[6] * t + page_b[6], height.idx[6]), height.age[6]);
}

vec4 img7(vec2 t)
{
    return age(tex(page_a[7] * t + page_b[7], height.idx[7]), height.age[7]);
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

    return face_M * normalize(vec3(x, y, z));
}

//------------------------------------------------------------------------------

void main()
{
    float k = sample(gl_Vertex.xy).r;
    float h = mix(r0, r1, k);
    vec3  v = h * scube(page_a[0] * gl_Vertex.xy + page_b[0]);

    var_L = gl_LightSource[0].position.xyz;
    var_V = v;

    gl_TexCoord[0].xy = gl_Vertex.xy;
    gl_FrontColor     = colormap(k);
    gl_Position       = gl_ModelViewProjectionMatrix * vec4(v, 1.0);
}
