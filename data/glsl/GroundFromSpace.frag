#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect dif;
uniform sampler2DRect nrm;

void main (void)
{
    vec3 L = gl_LightSource[0].position.xyz;
    vec3 D = texture2DRect(dif, gl_FragCoord.xy).rgb;
    vec3 N = texture2DRect(nrm, gl_FragCoord.xy).rgb;

    N = normalize((N * 2.0) - 1.0);

    vec3 d = max(dot(N, L), 0.0) * gl_LightSource[0].diffuse.rgb;
    vec3 a =                       gl_LightModel.ambient.rgb;

    gl_FragColor = gl_Color + vec4(D * (d + a), 1.0) * gl_SecondaryColor;
}
