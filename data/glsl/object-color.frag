
uniform sampler2D       spec_map;
uniform sampler2D       diff_map;
uniform sampler2D       norm_map;
uniform sampler2DShadow shadow[3];

uniform vec4 pssm_depth;

varying vec3 V_v;
varying vec3 L_v;

float get_shadow(sampler2DShadow sampler, vec4 coord)
{
    return shadow2DProj(sampler, coord).r;
}

/*
float shadow(sampler2DShadow sampler, vec4 coord)
{
    return shadow2D(sampler, coord.xyz / coord.w).r;
}
*/
/*
float shadow(sampler2DShadow sampler, vec4 coord)
{
    vec3 C = coord.xyz / coord.w;

    float s = 1.0 / 1024.0;

    vec3 d00 = vec3(-s, -s, 0.0);
    vec3 d01 = vec3(-s,  s, 0.0);
    vec3 d10 = vec3( s, -s, 0.0);
    vec3 d11 = vec3( s,  s, 0.0);

    vec4 S;

    S.r = shadow2D(sampler, C + d00).r;
    S.g = shadow2D(sampler, C + d01).r;
    S.b = shadow2D(sampler, C + d10).r;
    S.a = shadow2D(sampler, C + d11).r;

    return dot(S, vec4(0.25));
}
*/
void main()
{
    vec3 V = normalize(V_v);
    vec3 L = normalize(L_v);

    vec4  S  = texture2D(spec_map, gl_TexCoord[0].xy);
    vec4  D  = texture2D(diff_map, gl_TexCoord[0].xy);
    vec3  N  = texture2D(norm_map, gl_TexCoord[0].xy).rgb;
    float S0 = get_shadow(shadow[0], gl_TexCoord[1]);
    float S1 = get_shadow(shadow[1], gl_TexCoord[2]);
    float S2 = get_shadow(shadow[2], gl_TexCoord[3]);

    float kx = step(pssm_depth.y, gl_FragCoord.z);
    float ky = step(pssm_depth.z, gl_FragCoord.z);

    float lit = mix(S0, mix(S1, S2, ky), kx) * step(0.0, L.z);

    vec3 Z = mix(vec3(1.0, 0.0, 0.0),
                  mix(vec3(0.0, 1.0, 0.0),
                      vec3(0.0, 0.0, 1.0), ky), kx);

    N = normalize(2.0 * N - 1.0);

    vec3 R = reflect(L, N);

    vec3  Ka = vec3(0.5);
    vec3  Kd = vec3(0.5);
    vec3  Ks = vec3(1.0);
    float Ns = max(1.0, S.a * 64.0);

    vec3 KS = pow(max(dot(V, R), 0.0), Ns) * S.rgb * Ks;
    vec3 KD =     max(dot(L, N), 0.0)      * D.rgb * Kd;
    vec3 KA =                                D.rgb * Ka;

    gl_FragColor = vec4(((KS + KD) * lit + KA).rgb, D.a);
}
