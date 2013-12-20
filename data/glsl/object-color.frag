#version 120

uniform sampler2D       spec_map;
uniform sampler2D       diff_map;
uniform sampler2D       norm_map;

uniform sampler2DShadow shadow[4];

varying vec3 fV;
varying vec3 fL[4];
varying vec3 fD[4];
varying vec4 fS[4];

const vec3 Ka = vec3(0.2, 0.2, 0.2);

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

    return Td.rgb * kd + Ts.rgb * ks;
}

vec3 slight(vec3 V, vec3 N, vec4 Td, vec4 Ts, int i)
{
    float r =     length(fL[i]);
    vec3  L =  normalize(fL[i]);
    vec3  D = -normalize(fD[i]);

    float d = dot(L, D);

    float a  = 1.0 / (gl_LightSource[i].constantAttenuation  +
                      gl_LightSource[i].linearAttenuation    * r +
                      gl_LightSource[i].quadraticAttenuation * r * r);

    float s = step(gl_LightSource[i].spotCosCutoff, d) *
            pow(d, gl_LightSource[i].spotExponent);

    return a * s * shade(V, N, L, Td, Ts);
}

vec3 dlight(vec3 V, vec3 N, vec4 Td, vec4 Ts, int i)
{
    float n = gl_ClipPlane[0].w;
    float f = gl_ClipPlane[1].w;

    float z0 = splitz(gl_LightSource[i].ambient.x, n, f);
    float z1 = splitz(gl_LightSource[i].ambient.y, n, f);

    vec3 L = normalize(fL[i]);

    return shade(V, N, L, Td, Ts) * step(z0, gl_FragCoord.z)
                                  * step(gl_FragCoord.z, z1);
}

vec3 light(vec3 V, vec3 N, vec4 Td, vec4 Ts, int i)
{
    float S = shadow2DProj(shadow[i], fS[i]).r;
    vec3  C = gl_LightSource[i].diffuse.rgb;

    return C * S * mix(dlight(V, N, Td, Ts, i),
                       slight(V, N, Td, Ts, i), gl_LightSource[i].position.w);
}

void main()
{
    vec4 Td = texture2D(diff_map, gl_TexCoord[0].xy);
    vec4 Ts = texture2D(spec_map, gl_TexCoord[0].xy);
    vec4 Tn = texture2D(norm_map, gl_TexCoord[0].xy);

    vec3 V = normalize(fV);
    vec3 N = normalize(2.0 * Tn.rgb - 1.0);

    vec3 C = Ka * Td.rgb + light(V, N, Td, Ts, 0)
                         + light(V, N, Td, Ts, 1)
                         + light(V, N, Td, Ts, 2)
                         + light(V, N, Td, Ts, 3);

    gl_FragColor = vec4(C, Td.a);
}