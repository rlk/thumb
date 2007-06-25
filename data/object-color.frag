
uniform sampler2D       diffuse;
uniform sampler2D       bump;
uniform sampler2DShadow shadow;

varying vec3 V;
varying vec3 L;

void main()
{
    vec4  Kd = texture2D    (diffuse, gl_TexCoord[0].xy);
    vec3  N  = texture2D    (bump,    gl_TexCoord[0].xy).rgb;
    float S  =  shadow2DProj(shadow,  gl_TexCoord[1]).r;

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
