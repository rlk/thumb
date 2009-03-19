#extension GL_ARB_texture_rectangle : enable

uniform sampler2D       diff_map;
uniform sampler2D       spec_map;
uniform sampler2D       norm_map;

//uniform samplerCube     reflection_env;
//uniform samplerCube     irradiance_env;
uniform sampler2DRect   reflection_env[2];
uniform sampler2DRect   irradiance_env[2];

uniform sampler2DShadow shadow[3];
/*
uniform mat4 irradiance_R;
uniform mat4 irradiance_G;
uniform mat4 irradiance_B;
*/
uniform vec4 pssm_depth;
uniform vec3 light_position;

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

    // Grace Cathedral
/*    
    vec3 L0 = vec3( 0.79,  0.44,  0.54);
    vec3 L1 = vec3( 0.39,  0.35,  0.60);
    vec3 L2 = vec3(-0.34, -0.18, -0.27);
    vec3 L3 = vec3(-0.29, -0.06,  0.01);
    vec3 L4 = vec3(-0.11, -0.05, -0.12);
    vec3 L5 = vec3(-0.26, -0.22, -0.47);
    vec3 L6 = vec3(-0.16, -0.09, -0.15);
    vec3 L7 = vec3( 0.56,  0.21,  0.14);
    vec3 L8 = vec3( 0.21, -0.05, -0.30);
*/
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

    return E * 5.0;
}

float get_shadow(sampler2DShadow sampler, vec4 coord)
{
    return shadow2DProj(sampler, coord).r;
}

void main()
{
    // Normalize the varying world-space vectors.

    vec3 V_w = normalize(V_v);
    vec3 N_w = normalize(N_v);
    vec3 T_w = normalize(T_v);

    // TODO: uniform light_direction.
    // This is gonna cause problems.

    float lit = step(0.0, dot(N_w, normalize(light_position)));

    // Construct a tangent space basis in world space.

    mat3 M = mat3(T_w, normalize(cross(N_w, T_w)), N_w);

    // Look up the material textures.

    vec4 D_c = texture2D(diff_map, gl_TexCoord[0].xy);
    vec4 S_c = texture2D(spec_map, gl_TexCoord[0].xy);
    vec3 N_c = texture2D(norm_map, gl_TexCoord[0].xy).rgb;

    // Look up the shadow map textures.

    float S0 = get_shadow(shadow[0], gl_TexCoord[1]);
    float S1 = get_shadow(shadow[1], gl_TexCoord[2]);
    float S2 = get_shadow(shadow[2], gl_TexCoord[3]);

    float s0 = step(pssm_depth.y, gl_FragCoord.z);
    float s1 = step(pssm_depth.z, gl_FragCoord.z);
    float ss = mix(S0, mix(S1, S2, s1), s0) * lit;

    // Transform the fragment normal from tangent to world space.

    vec3 N = M * normalize(2.0 * N_c - 1.0);

    // Reflect the world-space view across the normal.

    vec3 R = reflect(V_w, N);

//  vec3 C = textureCube(reflection_env, R).rgb;

    vec4 C0 = vec4(irradiance(irradiance_env[0], N), 1.0);
    vec4 C1 = vec4(irradiance(irradiance_env[1], N), 1.0);

//  vec3 C = texture2DRect(irradiance_env, gl_TexCoord[0].xy * 3.0).rgb;

//  gl_FragColor = vec4(abs(C) * 4.0, 1.0);
//  gl_FragColor = mix(C0, C1, ss) * D_c;
    gl_FragColor = vec4(N_w * 0.5 + 0.5, 1.0);
}
