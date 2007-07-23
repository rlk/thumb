#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect cyl;
uniform sampler2DRect dif;
uniform sampler2DRect nrm;

varying vec3 L;

void main()
{
    vec4 C = texture2DRect(cyl, gl_FragCoord.xy);
    vec3 D = texture2DRect(dif, gl_FragCoord.xy).rgb;
    vec3 N = texture2DRect(nrm, gl_FragCoord.xy).rgb;

    N = normalize((N * 2.0) - 1.0);

    vec3 d = max(dot(N, L), 0.0) * gl_LightSource[0].diffuse.rgb;
    vec3 a =                       gl_LightModel.ambient.rgb;

//  gl_FragColor = vec4(D * d + a, 1.0);
    gl_FragColor = vec4(D * d, 1.0);
}
