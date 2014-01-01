#version 120

uniform vec2 LightSplit[4];
uniform vec2 LightBrightness[4];

uniform sampler2D       diffuse;
uniform sampler2D       specular;
uniform sampler2D       normal;

uniform sampler2DShadow shadow[4];
uniform sampler2D       cookie[4];

varying vec3 fC;
varying vec3 fV;
varying vec3 fL[4];
varying vec4 fS[4];

const vec3 Ka = vec3(0.4, 0.4, 0.4);

// Shadow map split coefficient, k in (0,1).

float splitc(float k, float n, float f)
{
    return mix(n * pow(f / n, k), n + (f - n) * k, 0.5);
}

// Shadow map split depth, k in (0,1).

float splitz(float k, float n, float f)
{
    float c = splitc(k, n, f);
    return (f / c) * (c - n) / (f - n);
}

// Phong shader.

vec3 phong(vec3 V, vec3 N, vec3 L, vec4 Td, vec4 Ts)
{
    vec3 R = reflect(L, N);

    float ks = pow(max(dot(V, R), 0.0), Ts.a * 64.0);
    float kd =     max(dot(L, N), 0.0);

    return Td.rgb * kd + Ts.rgb * ks;

    // return (Td.rgb * kd + Ts.rgb * ks) * step(0.0, L.z);
}

vec3 light(vec3 V, vec3 N, vec4 Td, vec4 Ts, int i)
{
    // Shadow and cookie

    float S =  shadow2DProj(shadow[i], fS[i]).r;
    vec3  C = texture2DProj(cookie[i], fS[i]).rgb * step(0.0, fS[i].q);

    // Attenuation coefficient

    vec3  L = normalize(fL[i]);
    float r =    length(fL[i]);
    float a = LightBrightness[i].x / max(1.0, LightBrightness[i].y * r);

    // Shadow map split coefficient

    float n = gl_ClipPlane[0].w;
    float f = gl_ClipPlane[1].w;

    float k = step(splitz(LightSplit[i].x, n, f), gl_FragCoord.z)
            * step(gl_FragCoord.z, splitz(LightSplit[i].y, n, f));

    // All coefficients modulate a Phong shading

    return C * S * a * k * phong(V, N, L, Td, Ts);
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

    gl_FragColor = vec4(C + fC, Td.a);
}