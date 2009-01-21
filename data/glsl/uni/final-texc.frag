#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect cyl;
uniform sampler2DRect dif;
uniform sampler2DRect nrm;

void main()
{
    vec4 C = texture2DRect(cyl, gl_FragCoord.xy);

    const float pi = 3.14159265358979323846;

//  const vec2 ck = vec2(0.5, 1.0) / pi;
    const vec2 ck = vec2(1.0, 1.0) / pi;
    const vec2 cd = vec2(1.0, 0.5);

//  gl_FragColor = vec4(C.xy * ck + cd, 0.0, 1.0);

//  const float M = 90.0;
//  const float M = 10010366.1190095366677902;
    const float M = 1001036.61190095366677902;

    vec2 c = fract((C.xy * ck + cd) * M);

    vec2 s = step(0.5, c);

    float k = s.x + s.y - s.x * s.y * 2.0;

    gl_FragColor = vec4(k, k, k, 1.0);
}
