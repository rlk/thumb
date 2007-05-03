
uniform sampler2D       diffuse;
uniform sampler2D       bump;
uniform sampler2D       light;
uniform sampler2DShadow shadow;

varying vec3 V;
varying vec3 L;
varying vec3 R;

void main()
{
    vec4  Kd = texture2D    (diffuse, gl_TexCoord[0].xy);
    vec3  N  = texture2D    (bump,    gl_TexCoord[0].xy).rgb;
    vec4  Kl = texture2DProj(light,   gl_TexCoord[1]);
    float S  =  shadow2DProj(shadow,  gl_TexCoord[1]).r;

    N = 2.0 * N - 1.0;

    vec3 R = reflect(L, N);

    Kd  = Kd * gl_FrontMaterial.diffuse;
    vec4  Ks = gl_FrontMaterial.specular;
    vec4  Ns = vec4(gl_FrontMaterial.shininess);

    // Clamp the range of the light source.
/*
    float s = gl_TexCoord[1].x / gl_TexCoord[1].w;
    float t = gl_TexCoord[1].y / gl_TexCoord[1].w;

    float c = step(0.0, gl_TexCoord[1].z) * step(0.0, s) * step(s, 1.0)
                                          * step(0.0, t) * step(t, 1.0);
*/
    vec4 l = Kl * S;

    vec4 KS = pow(max(dot(V, R), 0.0) * Ks * l, Ns);
    vec4 KD =     max(dot(L, N), 0.0) * Kd * l;

    gl_FragColor = vec4(KS.rgb + KD.rgb, KD.a);
}
