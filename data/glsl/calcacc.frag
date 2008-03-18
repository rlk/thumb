#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect src;
uniform sampler2D     map;
uniform sampler2DRect pos;
uniform sampler2DRect nrm;
uniform sampler2DRect tex;

uniform vec2 d;
uniform vec2 k;

void main()
{
    const float pi = 3.14159265358979323844;

    // Compute the local texture coordinate.

    vec2 T = texture2DRect(tex, gl_FragCoord.xy).xy;

    T.x -= 2.0 * pi * step(pi, T.x);
    T = T.xy * k + d;

    // Discard any pixel outside the current texture.

    vec2 a = step(vec2(0.0), T) * step(T, vec2(1.0));

    if (a.x * a.y < 1.0) discard;

//  const float off =     0.5;
//  const float scl =     1.0;
    const float off =     0.0;
    const float scl =    25.0;
    const float mag = 65535.0;

//  vec4 S = texture2DRect(src, gl_FragCoord.xy);
    vec3 P = texture2DRect(pos, gl_FragCoord.xy).xyz;
    vec3 N = texture2DRect(nrm, gl_FragCoord.xy).xyz;

    float s = texture2D(map, T).r;

    float M = (s - off) * scl * mag;

    vec4 D = vec4(P + N * M, M);

//    gl_FragColor = mix(S, D, a.x * a.y);
    gl_FragColor = D;
//    gl_FragColor = vec4(P, 0.0);
}
