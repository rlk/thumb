
uniform sampler2D       diffuse;
uniform sampler2D       bump;
uniform sampler2DShadow shadow0;
uniform sampler2DShadow shadow1;
uniform sampler2DShadow shadow2;

uniform vec4 split;

varying vec4 eye;
varying vec3 V;
varying vec3 L;
varying float G;

void main()
{
    vec4  Kd = texture2D    (diffuse, gl_TexCoord[0].xy);
    vec3  N  = texture2D    (bump,    gl_TexCoord[0].xy).rgb;
    float S0 =  shadow2DProj(shadow0, gl_TexCoord[1]).r;
    float S1 =  shadow2DProj(shadow1, gl_TexCoord[2]).r;
    float S2 =  shadow2DProj(shadow2, gl_TexCoord[3]).r;

    float kx = step(split.y, gl_FragCoord.z);
    float ky = step(split.z, gl_FragCoord.z);
    
    vec4 S = mix(S0, mix(S1, S2, ky), kx) * gl_LightSource[0].diffuse;

    S = S * step(0.0, G);

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
