
uniform sampler2D       spec_map;
uniform sampler2D       diff_map;
uniform sampler2D       norm_map;

uniform sampler2DShadow shadow[3];

varying vec3 V_v;
varying vec3 L_v;

const vec3 Kd = vec3(0.5, 0.5, 0.5);
const vec3 Ks = vec3(1.0, 1.0, 1.0);
const vec3 Ka = vec3(0.5, 0.5, 0.5);

float splitc(float k, float n, float f)
{
    return mix(n * pow(f / n, k), n + (f - n) * k, 0.5);
}

float splitz(float k, float n, float f)
{
    float c = splitc(k, n, f);
    return (f / c) * (c - n) / (f - n);
}

vec4 shadowSample(sampler2DShadow sampler, vec4 coord)
{
    return shadow2DProj(sampler, coord);
}

void main()
{
    float n = gl_ClipPlane[0].w;
    float f = gl_ClipPlane[1].w;

    vec3 V = normalize(V_v);
    vec3 L = normalize(L_v);

    vec4 Ts  = texture2D(spec_map, gl_TexCoord[0].xy);
    vec4 Td  = texture2D(diff_map, gl_TexCoord[0].xy);
    vec4 Tn  = texture2D(norm_map, gl_TexCoord[0].xy);

    float S0 = shadowSample(shadow[0], gl_TexCoord[1]).r;
    float S1 = shadowSample(shadow[1], gl_TexCoord[2]).r;
    float S2 = shadowSample(shadow[2], gl_TexCoord[3]).r;

    float S  = 1.0;

    float z3 = splitz(1.000, n, f);
    float z2 = splitz(0.666, n, f);
    float z1 = splitz(0.333, n, f);

    S = S0;

    // if (gl_FragCoord.z < z3) S = S2;
    // if (gl_FragCoord.z < z2) S = S1;
    // if (gl_FragCoord.z < z1) S = S0;

    vec4 K = vec4(1.0, 1.0, 1.0, 1.0);

//  if (gl_FragCoord.z < z3) K = vec4(0.5, 0.5, 1.0, 1.0);
//  if (gl_FragCoord.z < z2) K = vec4(0.5, 1.0, 0.5, 1.0);
//  if (gl_FragCoord.z < z1) K = vec4(1.0, 0.5, 0.5, 1.0);

    vec3 N = normalize(2.0 * Tn.rgb - 1.0);
    vec3 R = reflect(L, N);

    float Ns = max(1.0, Ts.a * 64.0);

    vec3 Cs = Ks * pow(max(dot(V, R), 0.0), Ns) * Ts.rgb;
    vec3 Cd = Kd *     max(dot(L, N), 0.0)      * Td.rgb;
    vec3 Ca = Ka *                                Td.rgb;

    gl_FragColor = vec4(mix(Ca, Cd + Cs + Ca, S), Td.a) * K;
}
