
uniform sampler2D       spec_map;
uniform sampler2D       diff_map;
uniform sampler2D       norm_map;

uniform sampler2DShadow shadow[3];

varying vec3 V_v;
varying vec3 L_v;

const vec3 Kd = vec3(0.5, 0.5, 0.5);
const vec3 Ks = vec3(1.0, 1.0, 1.0);
const vec3 Ka = vec3(0.5, 0.5, 0.5);

vec4 shadowSample(sampler2DShadow sampler, vec4 coord)
{
    return shadow2DProj(sampler, coord);
}

void main()
{
    vec3 V = normalize(V_v);
    vec3 L = normalize(L_v);

    vec4 Ts  = texture2D(spec_map, gl_TexCoord[0].xy);
    vec4 Td  = texture2D(diff_map, gl_TexCoord[0].xy);
    vec4 Tn  = texture2D(norm_map, gl_TexCoord[0].xy);

    float S0 = shadowSample(shadow[0], gl_TexCoord[1]).r;
    float S1 = shadowSample(shadow[1], gl_TexCoord[2]).r;
    float S2 = shadowSample(shadow[2], gl_TexCoord[3]).r;

    float S  = 1.0;

    if (gl_FragCoord.z < gl_ClipPlane[3].w) S = S2;
    if (gl_FragCoord.z < gl_ClipPlane[2].w) S = S1;
    if (gl_FragCoord.z < gl_ClipPlane[1].w) S = S0;

    // vec4 K = vec4(1.0, 1.0, 1.0, 1.0);

    // if (gl_FragCoord.z < gl_ClipPlane[3].w) K = vec4(0.5, 0.5, 1.0, 1.0);
    // if (gl_FragCoord.z < gl_ClipPlane[2].w) K = vec4(0.5, 1.0, 0.5, 1.0);
    // if (gl_FragCoord.z < gl_ClipPlane[1].w) K = vec4(1.0, 0.5, 0.5, 1.0);

    vec3 N = normalize(2.0 * Tn.rgb - 1.0);
    vec3 R = reflect(L, N);

    float Ns = max(1.0, Ts.a * 64.0);

    vec3 Cs = Ks * pow(max(dot(V, R), 0.0), Ns) * Ts.rgb;
    vec3 Cd = Kd *     max(dot(L, N), 0.0)      * Td.rgb;
    vec3 Ca = Ka *                                Td.rgb;

    gl_FragColor = vec4(mix(Ca, Cd + Cs + Ca, S), Td.a);
}
