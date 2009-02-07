
uniform sampler2D       diffuse;
uniform sampler2D       normal;
uniform sampler2DShadow shadow0;
uniform sampler2DShadow shadow1;
uniform sampler2DShadow shadow2;

uniform vec4 split;

varying vec3 V_v;
varying vec3 L_v;

void main()
{
    vec3 V = normalize(V_v);
    vec3 L = normalize(L_v);

    vec4  D  = texture2D(diffuse, gl_TexCoord[0].xy);
    vec3  N  = texture2D(normal,  gl_TexCoord[0].xy).rgb;
    float S0 = shadow2DProj(shadow0, gl_TexCoord[1]).r;
    float S1 = shadow2DProj(shadow1, gl_TexCoord[2]).r;
    float S2 = shadow2DProj(shadow2, gl_TexCoord[3]).r;

    float kx = step(split.y, gl_FragCoord.z);
    float ky = step(split.z, gl_FragCoord.z);

    float S = mix(S0, mix(S1, S2, ky), kx);


    vec3 Z = mix(vec3(1.0, 0.0, 0.0),
                  mix(vec3(0.0, 1.0, 0.0),
                      vec3(0.0, 0.0, 1.0), ky), kx);


    N = normalize(2.0 * N - 1.0);

    vec3 R = reflect(L, N);

    vec3  Ka = vec3(0.5);
    vec3  Kd = vec3(0.5);
    vec3  Ks = vec3(0.3);
    float Ns =      4.0;

    vec3 KS = pow(max(dot(V, R), 0.0), Ns)         * Ks;
    vec3 KD =     max(dot(L, N), 0.0)      * D.rgb * Kd;
    vec3 KA =                                D.rgb * Ka * Z;

    gl_FragColor = vec4(((KS + KD) * S + KA).rgb, D.a);
}
