#extension GL_ARB_texture_rectangle : enable

//-----------------------------------------------------------------------------

uniform sampler2DRect cyl;
uniform sampler2D     src;

uniform vec2 size;

//-----------------------------------------------------------------------------

float miplevel(vec2 p)
{
    vec2 dpdx = dFdx(p);
    vec2 dpdy = dFdy(p);

    return max(0.0, log2(max(sqrt(dpdx.s * dpdx.s + dpdx.t * dpdx.t),
                             sqrt(dpdy.s * dpdy.s + dpdy.t * dpdy.t))));
}

vec2 mipR(vec2 p, float l)
{
    const float t0 = exp2(l      );
    const float t1 = exp2(l + 1.0);

    return vec2(1.0 - 1.0 / t0 + p.s / t1,
                1.0 - 1.0 / t1 + p.t / t1);
}

vec2 mipG(vec2 p, float l)
{
    const float t0 = exp2(l      );
    const float t1 = exp2(l + 1.0);

    return vec2(1.0 - 1.0 / t0 + p.s / t1,
                1.0 - 1.0 / t0 + p.t / t1);
}

vec2 mipB(vec2 p, float l)
{
    const float t0 = exp2(l      );
    const float t1 = exp2(l + 1.0);

    return vec2(1.0 - 1.0 / t1 + p.s / t1,
                1.0 - 1.0 / t0 + p.t / t1);
}

void main()
{
    const float pi = 3.14159265358979323846;

    const vec2 ck = vec2(0.5, 1.0) / pi;
    const vec2 cd = vec2(0.5, 0.5);

    vec2 c = texture2DRect(cyl, gl_FragCoord.xy).xy * ck + cd;

    float l  = miplevel(c * size);
    float l0 = floor(l);
    float l1 = ceil (l);

    float k = l - l0;

    vec3 C0 = vec3(texture2D(src, mipR(c, l0)).r,
                   texture2D(src, mipG(c, l0)).r,
                   texture2D(src, mipB(c, l0)).r);
    vec3 C1 = vec3(texture2D(src, mipR(c, l1)).r,
                   texture2D(src, mipG(c, l1)).r,
                   texture2D(src, mipB(c, l1)).r);

//  gl_FragColor = vec4(mix(C0, C1, k), 1.0);
    gl_FragColor = vec4(C0, 1.0);
}

//-----------------------------------------------------------------------------

/*
uniform vec2 d;
uniform vec2 k;

void main()
{
    vec2 c = texture2DRect(cyl, gl_FragCoord.xy).xy;
    vec2 t = c * k + d;

    vec2  a = step(vec2(0.0), t) * step(t, vec2(1.0));
    float K = a.x * a.y;

    gl_FragColor = vec4(texture2D(src, t).rgb, K);
}
*/
