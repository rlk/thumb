#version 120

uniform vec2 LightSplit[4];

uniform sampler2D       diffuse;
uniform sampler2D       specular;
uniform sampler2D       normal;
  
uniform sampler2DShadow shadow[4];
uniform sampler2D       cookie[4];

varying vec3 fV;
varying vec3 fL[4];
varying vec4 fS[4];

const vec3 Ka = vec3(0.4, 0.4, 0.4);

float splitc(float k, float n, float f)
{
    return mix(n * pow(f / n, k), n + (f - n) * k, 0.5);
}

float splitz(float k, float n, float f)
{
    float c = splitc(k, n, f);
    return (f / c) * (c - n) / (f - n);
}

vec3 shade(vec3 V, vec3 N, vec3 L, vec4 Td, vec4 Ts)
{
    vec3 R = reflect(L, N);

    float ks = pow(max(dot(V, R), 0.0), Ts.a * 64.0);
    float kd =     max(dot(L, N), 0.0);

    // return (Td.rgb * kd + Ts.rgb * ks) * step(0.0, L.z);
    return Td.rgb * kd + Ts.rgb * ks;
}

vec3 light(vec3 V, vec3 N, vec4 Td, vec4 Ts, int i)
{
    float S =  shadow2DProj(shadow[i], fS[i]).r;
    vec3  C = texture2DProj(cookie[i], fS[i]).rgb * step(0.0, fS[i].q);

    vec3  L = normalize(fL[i]);

    float n = gl_ClipPlane[0].w;
    float f = gl_ClipPlane[1].w;

    float z0 = splitz(LightSplit[i].x, n, f);
    float z1 = splitz(LightSplit[i].y, n, f);

    return C * S * shade(V, N, L, Td, Ts) * step(z0, gl_FragCoord.z)
                                          * step(gl_FragCoord.z, z1);
}

void main()
{
    vec4 Td = texture2D(diffuse,  gl_TexCoord[0].xy);
    vec4 Ts = texture2D(specular, gl_TexCoord[0].xy);
    vec4 Tn = texture2D(normal,   gl_TexCoord[0].xy);

    vec3 V = normalize(-fV);
    vec3 N = normalize(2.0 * Tn.rgb - 1.0);

    vec3 C = Ka * Td.rgb + light(V, N, Td, Ts, 0)
                         + light(V, N, Td, Ts, 1)
                         + light(V, N, Td, Ts, 2)
                         + light(V, N, Td, Ts, 3);

    gl_FragColor = vec4(C, Td.a);
}