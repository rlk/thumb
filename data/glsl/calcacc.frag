#extension GL_ARB_texture_rectangle : enable

#define FILTER 1

uniform sampler2DRect src;
uniform sampler2D     map;
uniform sampler2DRect pos;
uniform sampler2DRect nrm;
uniform sampler2DRect tex;

void main()
{
    const float dds = 1.0 / 4096.0;
    const float ddt = 1.0 / 2048.0;

    const float off = 0.0862745098039215;
    const float scl = 1.1697247706422018;
    const float mag = 10345.045871559634;

    vec3 S = texture2DRect(src, gl_FragCoord.xy).xyz;
    vec3 P = texture2DRect(pos, gl_FragCoord.xy).xyz;
    vec3 N = texture2DRect(nrm, gl_FragCoord.xy).xyz;
    vec2 T = texture2DRect(tex, gl_FragCoord.xy).zw;

    vec2 uv = fract(T * vec2(4096.0, 2048.0));

#if FILTER
    float c00 = texture2D(map, T + vec2(0.0, 0.0)).r;
    float c10 = texture2D(map, T + vec2(dds, 0.0)).r;
    float c01 = texture2D(map, T + vec2(0.0, ddt)).r;
    float c11 = texture2D(map, T + vec2(dds, ddt)).r;

    float s  = mix(mix(c00, c10, uv.x),
                   mix(c01, c11, uv.x), uv.y);
#else
    float s = texture2D(map, T).r;
#endif

    float M = (s - off) * scl * mag;

    gl_FragColor = vec4(P + N * M, M);
//  gl_FragColor = vec4(P, M);
}
