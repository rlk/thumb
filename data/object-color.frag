
uniform sampler2D       diffuse;
uniform sampler2D       normal;
uniform sampler2DShadow shadow0;
uniform sampler2DShadow shadow1;
uniform sampler2DShadow shadow2;

uniform vec4 split;

varying vec3 V_v;
varying vec3 L_v;
/*
varying vec3 N_v;
varying vec3 T_v;
*/
void main()
{
    // Compute the orthonormalized tangent space basis.
/*
    mat3 T = mat3(normalize(T_v),
                  normalize(cross(N_v, T_v)),
                  normalize(N_v));
*/
/*
    mat3 T;

    T[0] = normalize(T_v);
    T[2] = normalize(N_v);
    T[1] = normalize(cross(T[2], T[0]));
*/
    // Compute the tangent space view and light vectors.
/*
    vec3 V = normalize(V_v.xyz) * T;
    vec3 L = normalize(L_v.xyz) * T;
*/
    vec3 V = normalize(V_v);
    vec3 L = normalize(L_v);

    vec4 Kd = texture2D(diffuse, gl_TexCoord[0].xy);
    vec3 N  = texture2D(normal,  gl_TexCoord[0].xy).rgb;

    N = normalize(2.0 * N - 1.0);

    vec3 R = reflect(L, N);

    vec4  Ks = vec4(0.5);
    float Ns =      4.0;

    vec4 KS = pow(max(dot(V, R), 0.0), Ns) * Ks;
    vec4 KD =     max(dot(L, N), 0.0)      * Kd;
    vec4 KA =        gl_LightModel.ambient * Kd;

//  gl_FragColor = vec4(((KS + KD) * S + KA).rgb, KD.a);
    gl_FragColor = vec4(((KS + KD) + KA).rgb, Kd.a);
//  gl_FragColor = vec4(((KS + KD)).rgb, Kd.a);
//  gl_FragColor = vec4((KD).rgb, 1.0);
}
/*
    float S0 =  shadow2DProj(shadow0, gl_TexCoord[1]).r;
    float S1 =  shadow2DProj(shadow1, gl_TexCoord[2]).r;
    float S2 =  shadow2DProj(shadow2, gl_TexCoord[3]).r;

    float kx = step(split.y, gl_FragCoord.z);
    float ky = step(split.z, gl_FragCoord.z);
    
    vec4 S = mix(S0, mix(S1, S2, ky), kx) * gl_LightSource[0].diffuse;

    S = S * step(0.0, G);
*/
