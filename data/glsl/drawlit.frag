#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect cyl;
uniform sampler2DRect dif;
uniform sampler2DRect nrm;

varying vec3 L;

void main()
{
//  vec4 C = texture2DRect(cyl, gl_FragCoord.xy);
    vec4 D = texture2DRect(dif, gl_FragCoord.xy);
    vec4 N = texture2DRect(nrm, gl_FragCoord.xy);

//  vec4 Cx = texture2DRect(cyl, gl_FragCoord.xy + vec2(1.0, 0.0));
//  vec4 Cy = texture2DRect(cyl, gl_FragCoord.xy + vec2(0.0, 1.0));

//  D.rgb = D.rgb / D.a;
//  N.rgb = N.rgb / N.a;

    vec3 n = normalize((N.rgb * 2.0) - 1.0);

    vec3 d = max(dot(n, L), 0.0) * gl_LightSource[0].diffuse.rgb;
    vec3 a =                       gl_LightModel.ambient.rgb;

    gl_FragColor = vec4(D.rgb * min(d + a, 1.0), 1.0);
//  gl_FragColor = vec4(D.rgb, 1.0);
//  gl_FragColor = vec4(d, 1.0);
//  gl_FragColor = vec4((n + 1.0) * 0.5, 1.0);
//  gl_FragColor = vec4(vec3(D.a), 1.0);
/*
    const float pi = 3.14159265358979323846;

    const vec2 ck = vec2(0.5, 1.0) / pi;
    const vec2 cd = vec2(0.5, 0.5);

    gl_FragColor = vec4(C.xy * ck + cd, 0.0, 1.0);
*/
}
