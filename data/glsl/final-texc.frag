#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect cyl;
uniform sampler2DRect dif;
uniform sampler2DRect nrm;

void main()
{
    vec4 C = texture2DRect(cyl, gl_FragCoord.xy);

    const float pi = 3.14159265358979323846;

    const vec2 ck = vec2(0.5, 1.0) / pi;
    const vec2 cd = vec2(0.5, 0.5);

    gl_FragColor = vec4(C.xy * ck + cd, 0.0, 1.0);
}
