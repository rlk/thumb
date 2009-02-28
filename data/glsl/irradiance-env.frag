
uniform sampler2D       diff_map;
uniform sampler2D       spec_map;
uniform sampler2D       norm_map;
/*
uniform samplerCube     diff_env;
*/
uniform samplerCube     spec_env;
uniform sampler2DShadow shadow[3];

uniform vec4 pssm_depth;

varying vec3 V_v;
varying vec3 N_v;
varying vec3 T_v;

float shadow(sampler2DShadow sampler, vec4 coord)
{
    return shadow2DProj(sampler, coord).r;
}

void main()
{
    // Normalize the varying world-space vectors.

    vec3 V_w = normalize(V_v);
    vec3 N_w = normalize(N_v);
    vec3 T_w = normalize(T_v);

    // Construct a tangent space basis in world space.

    mat3 M = mat3(T_w, normalize(cross(N_w, T_w)), N_w);

    // Look up the material textures.

    vec4 D_c = texture2D(diff_map, gl_TexCoord[0].xy);
    vec4 S_c = texture2D(spec_map, gl_TexCoord[0].xy);
    vec3 N_c = texture2D(norm_map, gl_TexCoord[0].xy).rgb;

    // Look up the shadow map textures.

    float S0 = shadow(shadow[0], gl_TexCoord[1]);
    float S1 = shadow(shadow[1], gl_TexCoord[2]);
    float S2 = shadow(shadow[2], gl_TexCoord[3]);

    float s0 = step(pssm_depth.y, gl_FragCoord.z);
    float s1 = step(pssm_depth.z, gl_FragCoord.z);
    float ss = mix(S0, mix(S1, S2, s1), s0);

    // Transform the fragment normal from tangent to world space.

    vec3 N = M * normalize(2.0 * N_c - 1.0);

    // Reflect the world-space view across the normal.

    vec3 R = reflect(V_w, N);

    gl_FragColor = textureCube(spec_env, R);
//  gl_FragColor = textureCube(spec_env, N);
}
