#extension GL_ARB_texture_rectangle : enable

#define FILTER 0

uniform sampler2DRect src;
uniform sampler2D     map;
uniform sampler2DRect pos;
uniform sampler2DRect nrm;
uniform sampler2DRect tex;

void main()
{
    const float off =     0.5;
    const float scl =     1.0;
    const float mag = 65535.0;

    vec4 S = texture2DRect(src, gl_FragCoord.xy);
    vec3 P = texture2DRect(pos, gl_FragCoord.xy).xyz;
    vec3 N = texture2DRect(nrm, gl_FragCoord.xy).xyz;
    vec2 T = texture2DRect(tex, gl_FragCoord.xy).xy;

    T = (gl_TextureMatrix[1] * vec4(T, 0.0, 1.0)).xy;

#if FILTER
    const float dds = 1.0 / 1024.0;
    const float ddt = 1.0 / 1024.0;

    vec2 uv = fract(T * vec2(1024.0, 1024.0));

    float c00 = texture2D(map, T + vec2(0.0, 0.0)).r;
    float c10 = texture2D(map, T + vec2(dds, 0.0)).r;
    float c01 = texture2D(map, T + vec2(0.0, ddt)).r;
    float c11 = texture2D(map, T + vec2(dds, ddt)).r;

    float s  = mix(mix(c00, c10, uv.x),
                   mix(c01, c11, uv.x), uv.y);
#else
    float s = texture2D(map, T).r;
#endif

    vec2 a = step(vec2(0.0), T) * step(T, vec2(1.0));

    float M = (s - off) * scl * mag;

    vec4 D = vec4(P + N * M, M);

//    gl_FragColor = mix(S, D, a.x * a.y);
    gl_FragColor = D;
}
