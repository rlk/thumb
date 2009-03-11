#extension GL_ARB_texture_rectangle : enable

uniform sampler2D       diff_map;
uniform sampler2D       spec_map;
uniform sampler2D       norm_map;

uniform samplerCube     reflection_env;
//niform samplerCube     irradiance_env;
uniform sampler2DRect   irradiance_env;

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
/*
    float S0 = get_shadow(shadow[0], gl_TexCoord[1]);
    float S1 = get_shadow(shadow[1], gl_TexCoord[2]);
    float S2 = get_shadow(shadow[2], gl_TexCoord[3]);

    float s0 = step(pssm_depth.y, gl_FragCoord.z);
    float s1 = step(pssm_depth.z, gl_FragCoord.z);
    float ss = mix(S0, mix(S1, S2, s1), s0) * lit;
*/
    // Transform the fragment normal from tangent to world space.

    vec3 N = M * normalize(2.0 * N_c - 1.0);

    // Reflect the world-space view across the normal.

    vec3 R = reflect(V_w, N);
/*
    float x = textureCube(irradiance_env, N).r * 2048.0 * 12.0;

    vec3 CC = (mix(vec3(0.0, 0.0, 0.0), 
                   vec3(0.0, 1.0, 0.0), clamp( x, 0.0, 1.0)) +
               mix(vec3(0.0, 0.0, 0.0), 
                   vec3(1.0, 0.0, 0.0), clamp(-x, 0.0, 1.0)));
*/
//  gl_FragColor = textureCube(reflection_env, R);

    vec3 C = texture2DRect(irradiance_env, gl_TexCoord[0].xy * 128.0 * 3.0).rgb;

    gl_FragColor = vec4(C * 12.0 * 4096.0, 1.0);
//  gl_FragColor = vec4(C, 1.0);
}
