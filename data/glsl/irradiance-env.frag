
uniform sampler2D       mat_diffuse;
uniform sampler2D       mat_specular;
uniform sampler2D       mat_normal;
uniform samplerCube     env_diffuse;
uniform sampler2DShadow shadow0;
uniform sampler2DShadow shadow1;
uniform sampler2DShadow shadow2;

uniform vec4 split;

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

    vec4  D_c = texture2D(mat_diffuse,  gl_TexCoord[0].xy);
    vec4  S_c = texture2D(mat_specular, gl_TexCoord[0].xy);
    vec3  N_c = texture2D(mat_normal,   gl_TexCoord[0].xy).rgb;

    // Look up the shadow map textures.

    float S0  = shadow(shadow0, gl_TexCoord[1]);
    float S1  = shadow(shadow1, gl_TexCoord[2]);
    float S2  = shadow(shadow2, gl_TexCoord[3]);

    float s0 = step(split.z, gl_FragCoord.z);
    float s1 = step(split.y, gl_FragCoord.z);
    float ss = mix(S0, mix(S1, S2, s1), s0);

    // Transform the fragment normal from tangent to world space.

    vec3 N = M * normalize(2.0 * N_c - 1.0);

    // Reflect the world-space view across the normal.

    vec3 R = reflect(V_w, N);

//  gl_FragColor = textureCube(env_diffuse, N);
    gl_FragColor = textureCube(env_diffuse, R);
}
