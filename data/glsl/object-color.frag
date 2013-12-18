#version 120

uniform sampler2D       spec_map;
uniform sampler2D       diff_map;
uniform sampler2D       norm_map;

uniform sampler2DShadow shadow[3];

varying vec4 fV;
varying vec4 fL[4];
varying vec3 fD[4];
varying vec4 fS[4];

const vec3 Kd = vec3(0.5, 0.5, 0.5);
const vec3 Ks = vec3(1.0, 1.0, 1.0);
const vec3 Ka = vec3(0.0, 0.0, 0.0);

float splitc(float k, float n, float f)
{
    return mix(n * pow(f / n, k), n + (f - n) * k, 0.5);
}

float splitz(float k, float n, float f)
{
    float c = splitc(k, n, f);
    return (f / c) * (c - n) / (f - n);
}

void main()
{
    float n = gl_ClipPlane[0].w;
    float f = gl_ClipPlane[1].w;

    vec4 Ts = texture2D(spec_map, gl_TexCoord[0].xy);
    vec4 Td = texture2D(diff_map, gl_TexCoord[0].xy);
    vec4 Tn = texture2D(norm_map, gl_TexCoord[0].xy);

    vec3  V = normalize(-fV.xyz);
    vec3  N = normalize(2.0 * Tn.rgb - 1.0);

    vec3 Ca = Ka * Td.rgb;
    vec3 Cd = vec3(0.0);
    vec3 Cs = vec3(0.0);

    for (int i = 0; i < 1; i++)
    {
        // float k0 = gl_LightSource[i].ambient.x;
        // float k1 = gl_LightSource[i].ambient.y;

        float S = shadow2DProj(shadow[i], fS[i]).r;

        vec3  L = normalize(fL[i].xyz - fV.xyz);
        vec3  R = reflect(L, N);

        float k = step(0.0, L.z);

        float Ns = max(1.0, Ts.a * 64.0);

        Cs += S * k * Ks * pow(max(dot(V, R), 0.0), Ns) * Ts.rgb;
        Cd += S * k * Kd *     max(dot(L, N), 0.0)      * Td.rgb;
    }

    // float S  = 1.0;

    // float z3 = splitz(1.000, n, f);
    // float z2 = splitz(0.666, n, f);
    // float z1 = splitz(0.333, n, f);

    // S = S0;

    // if (gl_FragCoord.z < z3) S = S2;
    // if (gl_FragCoord.z < z2) S = S1;
    // if (gl_FragCoord.z < z1) S = S0;

    gl_FragColor = vec4(Ca + Cd + Cs, Td.a);
}
