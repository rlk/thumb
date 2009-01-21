#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect cyl;
uniform sampler2DRect dif;
uniform sampler2DRect nrm;

varying vec3 L;

void main()
{
    vec4 D = texture2DRect(dif, gl_FragCoord.xy);
    vec4 N = texture2DRect(nrm, gl_FragCoord.xy);

    vec3 n = normalize((N.rgb * 2.0) - 1.0);

    vec3 d = max(dot(n, L), 0.0) * gl_LightSource[0].diffuse.rgb;

    gl_FragColor = vec4(d, 1.0);
}
