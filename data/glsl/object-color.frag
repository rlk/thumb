
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

    vec4 Kd = texture2D(diffuse, gl_TexCoord[0].xy);
    vec3  N = texture2D(normal,  gl_TexCoord[0].xy).rgb;
    float S = shadow2DProj(shadow0, gl_TexCoord[1]).r;

    N = normalize(2.0 * N - 1.0);

    vec3 R = reflect(L, N);

    vec4  Ks = vec4(0.5);
    float Ns =      4.0;

    vec4 KS = pow(max(dot(V, R), 0.0), Ns) * Ks;
    vec4 KD =     max(dot(L, N), 0.0)      * Kd;
    vec4 KA =        gl_LightModel.ambient * Kd;

    gl_FragColor = vec4(((KS + KD) * S + KA).rgb, Kd.a);
//    gl_FragColor = vec4((KS + KD + KA).rgb, Kd.a);
}
