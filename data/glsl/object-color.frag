
uniform sampler2D       spec_map;
uniform sampler2D       diff_map;
uniform sampler2D       norm_map;

uniform sampler2DShadow shadow[3];

varying vec3 V_v;
varying vec3 L_v;


vec4 shadowSample(sampler2DShadow sampler, vec4 coord)
{
    return shadow2DProj(sampler, coord);
}

void main()
{
    vec3 V = normalize(V_v);
    vec3 L = normalize(L_v);

    vec4  S  = texture2D(spec_map, gl_TexCoord[0].xy);
    vec4  D  = texture2D(diff_map, gl_TexCoord[0].xy);
    vec3  N  = texture2D(norm_map, gl_TexCoord[0].xy).rgb;

    float S0 = shadowSample(shadow[0], gl_TexCoord[1]).r;
    float S1 = shadowSample(shadow[1], gl_TexCoord[2]).r;
    float S2 = shadowSample(shadow[2], gl_TexCoord[3]).r;

    // float lit = mix(S0, mix(S1, S2, ky), kx) * step(0.0, L.z);

    vec4 K = vec4(0.0, 0.0, 0.0, 1.0);

    if (gl_FragCoord.z < gl_ClipPlane[3].w) K = vec4(0.5, 0.5, 1.0, 1.0);
    if (gl_FragCoord.z < gl_ClipPlane[2].w) K = vec4(0.5, 1.0, 0.5, 1.0);
    if (gl_FragCoord.z < gl_ClipPlane[1].w) K = vec4(1.0, 0.5, 0.5, 1.0);

    float lit = 1.0;

    N = normalize(2.0 * N - 1.0);

    vec3 R = reflect(L, N);

    vec3  Ka = vec3(0.5);
    vec3  Kd = vec3(0.5);
    vec3  Ks = vec3(1.0);
    float Ns = max(1.0, S.a * 64.0);

    vec3 KS = pow(max(dot(V, R), 0.0), Ns) * S.rgb * Ks;
    vec3 KD =     max(dot(L, N), 0.0)      * D.rgb * Kd;
    vec3 KA =                                D.rgb * Ka;

    gl_FragColor = K * vec4(((KS + KD) * lit + KA).rgb, D.a);
}
