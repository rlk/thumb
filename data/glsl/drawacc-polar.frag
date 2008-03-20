#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect src;
uniform sampler2D     map;
uniform sampler2DRect pos;
uniform sampler2DRect nrm;
uniform sampler2DRect tex;

uniform vec2 d;
uniform vec2 k;

uniform float up;

void main()
{
    const float pi = 3.14159265358979323844;

    // Compute the local texture coordinate.

    vec4 c = texture2DRect(tex, gl_FragCoord.xy);
    c.w = -c.w;
    vec2 t = (c.zw + 1.0) * 0.5;

    // Discard any pixel outside the current texture.

    vec2 a = step(vec2(0.0), t) * step(t, vec2(1.0));

//    if (a.x * a.y < 1.0) discard;

    float q = max(0.0, up * sin(c.y));
    q = q * q * (3.0 - 2.0 * q);

//  const float off =     0.5;
//  const float scl =     1.0;
    const float off =     0.0;
    const float scl =    25.0;
    const float mag = 65535.0;

    vec4 S = texture2DRect(src, gl_FragCoord.xy);
    vec3 P = texture2DRect(pos, gl_FragCoord.xy).xyz;
    vec3 N = texture2DRect(nrm, gl_FragCoord.xy).xyz;

    float s = texture2D(map, t).r;
    
    float M0 = S.a;
    float M1 = (s - off) * scl * mag;

    float M = M0 + M1 * q;

    vec4 D = vec4(P + N * M, M);

    gl_FragColor = D;
//    gl_FragColor = mix(S, D, a.x * a.y);
//    gl_FragColor = vec4(P, 0.0);
}
