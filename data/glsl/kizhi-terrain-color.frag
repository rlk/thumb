#extension GL_ARB_texture_rectangle : enable

uniform vec3 terrain_size;

uniform sampler2D       diff_map;
uniform sampler2D       spec_map;
uniform sampler2D       norm_map;

uniform sampler2DRect   reflection_env[2];
uniform sampler2DRect   irradiance_env[2];

uniform sampler2DShadow shadow[3];

uniform vec4 pssm_depth;
uniform vec3 light_position;
uniform vec4 color_max;

varying vec3 V_v;
varying vec3 N_v;
varying vec3 T_v;

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

void main()
{
    // Look up the material textures.

    vec4 D_c = texture2D(diff_map, gl_TexCoord[0].xy);
    vec4 S_c = texture2D(spec_map, gl_TexCoord[0].xy);
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

    gl_FragColor = mix(C0, C1, ss) * max(D_c, color_max);
}
