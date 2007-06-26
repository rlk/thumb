
uniform sampler2D       diffuse;
uniform sampler2D       bump;
uniform sampler2DShadow shadow0;
uniform sampler2DShadow shadow1;
uniform sampler2DShadow shadow2;

varying vec3 V;
varying vec3 L;

void main()
{
    vec4  Kd = texture2D    (diffuse, gl_TexCoord[0].xy);
    vec3  N  = texture2D    (bump,    gl_TexCoord[0].xy).rgb;
    float S0 =  shadow2DProj(shadow0, gl_TexCoord[1]).r;
    float S1 =  shadow2DProj(shadow1, gl_TexCoord[2]).r;
    float S2 =  shadow2DProj(shadow2, gl_TexCoord[3]).r;

//    float S = clamp(S0 + S1 + S2, 0.0, 1.0);

    float S = S1;

    N = normalize(2.0 * N - 1.0);

    vec3 R = reflect(L, N);

    Kd  = Kd * vec4(1.0);
    vec4  Ks = vec4(1.0);
    vec4  Ns = vec4(2.0);

    vec4 KS = pow(max(dot(V, R), 0.0) * Ks, Ns);
    vec4 KD =     max(dot(L, N), 0.0) * Kd;
    vec4 KA =   gl_LightModel.ambient * Kd;

    gl_FragColor = vec4(((KS + KD) * S + KA).rgb, KD.a);
}