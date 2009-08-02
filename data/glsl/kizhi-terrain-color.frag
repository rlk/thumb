#extension GL_ARB_texture_rectangle : enable

uniform vec3 terrain_size;

uniform sampler2D diff_map;
uniform sampler2D norm_map;

uniform sampler2D diff_0;
uniform sampler2D diff_1;
uniform sampler2D diff_2;
uniform sampler2D diff_3;
uniform sampler2D diff_4;
uniform sampler2D diff_5;
uniform sampler2D diff_6;
uniform sampler2D diff_7;

uniform sampler2DRect reflection_env[2];
uniform sampler2DRect irradiance_env[2];

uniform sampler2DShadow shadow[3];

uniform vec4 pssm_depth;
uniform vec3 light_position;
uniform vec4 color_max;

varying vec4 V_e;
varying vec3 V_v;

vec3 irradiance(sampler2DRect env, vec3 N)
{
    const float c1 = 0.429043;
    const float c2 = 0.511664;
    const float c3 = 0.743125;
    const float c4 = 0.886227;
    const float c5 = 0.247708;

    vec3 L0 = texture2DRect(env, vec2(0.5, 0.5)).rgb; // L0 0 = 0
    vec3 L1 = texture2DRect(env, vec2(1.5, 0.5)).rgb; // L1-1 = 1
    vec3 L2 = texture2DRect(env, vec2(2.5, 0.5)).rgb; // L1 0 = 2
    vec3 L3 = texture2DRect(env, vec2(0.5, 1.5)).rgb; // L1 1 = 3
    vec3 L4 = texture2DRect(env, vec2(1.5, 1.5)).rgb; // L2-2 = 4
    vec3 L5 = texture2DRect(env, vec2(2.5, 1.5)).rgb; // L2-1 = 5
    vec3 L6 = texture2DRect(env, vec2(0.5, 2.5)).rgb; // L2 0 = 6
    vec3 L7 = texture2DRect(env, vec2(1.5, 2.5)).rgb; // L2 1 = 7
    vec3 L8 = texture2DRect(env, vec2(2.5, 2.5)).rgb; // L2 2 = 8

    float x = N.x;
    float y = N.z;
    float z = N.y;

    vec3 E;

    E   =       c1 *  L8 * (x * x - y * y)
        +       c3 *  L6 * (z * z)
        +       c4 *  L0
        -       c5 *  L6
        + 2.0 * c1 * (L4 * x * y + L7 * x * z + L5 * y * z)
        + 2.0 * c2 * (L3 * x     + L1 * y     + L2 * z    );

    return E * 4.0;
}

float get_shadow(sampler2DShadow sampler, vec4 coord)
{
    return shadow2DProj(sampler, coord).r;
}

vec4 water(vec4 color, float depth)
{
    vec4 water = vec4(0.03, 0.15, 0.15, 1.0);

    float k = clamp(-1.0 * depth, 0.0, 1.0);
    float a = clamp( 0.9 - k,     0.0, 1.0);

    return vec4(mix(color.rgb, water.rgb, k), a);
}

void main()
{
    const float w = 2048.0;
    const float h = 4096.0;

    const vec2 d00 = vec2(0.0,     0.0    );
    const vec2 d01 = vec2(0.0,     1.0 / h);
    const vec2 d10 = vec2(1.0 / w, 0.0    );
    const vec2 d11 = vec2(1.0 / w, 1.0 / h);

    // Look up the diffuse color.

    vec2 k = fract(gl_TexCoord[0].xy * vec2(w, h));

    vec4 K_00 = texture2D(diff_map, gl_TexCoord[0].xy + d00);
    vec4 K_01 = texture2D(diff_map, gl_TexCoord[0].xy + d01);
    vec4 K_10 = texture2D(diff_map, gl_TexCoord[0].xy + d10);
    vec4 K_11 = texture2D(diff_map, gl_TexCoord[0].xy + d11);

    vec4 A_00 = fract(255.0 * K_00 / 16.0);
    vec4 A_01 = fract(255.0 * K_01 / 16.0);
    vec4 A_10 = fract(255.0 * K_10 / 16.0);
    vec4 A_11 = fract(255.0 * K_11 / 16.0);

    vec4 B_00 = floor(255.0 * K_00 / 16.0) / 15.0;
    vec4 B_01 = floor(255.0 * K_01 / 16.0) / 15.0;
    vec4 B_10 = floor(255.0 * K_10 / 16.0) / 15.0;
    vec4 B_11 = floor(255.0 * K_11 / 16.0) / 15.0;

    vec4 D_a = mix(mix(A_00, A_01, k.y), mix(A_10, A_11, k.y), k.x);
    vec4 D_b = mix(mix(B_00, B_01, k.y), mix(B_10, B_11, k.y), k.x);

    float total = min(1.0, dot(D_a, vec4(1.0)) + dot(D_b, vec4(1.0)));

    vec2 T = gl_TexCoord[0].xy * 512.0;

    vec3 D_0 = texture2D(diff_0, T).rgb;
    vec3 D_1 = texture2D(diff_1, T).rgb;
//  vec3 D_2 = texture2D(diff_2, T).rgb;
    vec3 D_3 = texture2D(diff_3, T).rgb;
    vec3 D_4 = texture2D(diff_4, T).rgb;
//  vec3 D_5 = texture2D(diff_5, T).rgb;
    vec3 D_6 = texture2D(diff_6, T).rgb;
    vec3 D_7 = texture2D(diff_7, T).rgb;

    vec3 D_c = mix(mix(mix(mix(mix(D_6, D_7,
                                   D_b.b),
                               D_4, D_b.r),
                           D_3, D_a.a),
                       D_1, D_a.g),
                   D_0, D_a.r);
/*
    vec3 D_c = mix(D_7, D_0 * D_a.r +
                        D_1 * D_a.g +
//                      D_2 * D_a.b +
                        D_3 * D_a.a +
                        D_4 * D_b.r +
//                      D_5 * D_b.g +
                        D_6 * D_b.b +
                        D_7 * D_b.a, total);
*/
    // Look up the normal map.

    vec3 N_c = texture2D(norm_map, gl_TexCoord[0].xy).rgb;

    vec3 N_w = normalize(2.0 * N_c - 1.0);

    // Look up the shadow map textures.

    float lit = step(0.0, dot(N_w, normalize(light_position)));

    float S0 = get_shadow(shadow[0], gl_TexCoord[1]);
    float S1 = get_shadow(shadow[1], gl_TexCoord[2]);
    float S2 = get_shadow(shadow[2], gl_TexCoord[3]);

    float s0 = step(pssm_depth.y, gl_FragCoord.z);
    float s1 = step(pssm_depth.z, gl_FragCoord.z);
    float ss = mix(S0, mix(S1, S2, s1), s0) * lit;

    // Sample the irradiance environment.

    vec4 C0 = vec4(irradiance(irradiance_env[0], N_w), 1.0);
    vec4 C1 = vec4(irradiance(irradiance_env[1], N_w), 1.0);

    vec4 C = mix(C0, C1, ss) * vec4(D_c, 1.0);

    float d = max(0.0, 1.0 + 0.0005 * V_e.z);

    if (V_v.y < 0.0) C = water(C, V_v.y);

    gl_FragColor = vec4(C.rgb, min(C.a, d));
}
