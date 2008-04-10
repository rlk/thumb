#extension GL_ARB_texture_rectangle : enable

//-----------------------------------------------------------------------------

uniform sampler2DRect cyl;
uniform sampler2D     index;
uniform sampler2D     cache;

uniform vec2 data_size;
uniform vec2 page_size;

//-----------------------------------------------------------------------------

float miplev(vec2 p)
{
    const vec2 dpdx = dFdx(p);
    const vec2 dpdy = dFdy(p);

    float rho = length(vec2(length(dpdx),
                            length(dpdy)));

    return clamp(log2(rho), 0.0, 7.0);
}

vec3 mipref(vec2 c, float l)
{
    const vec2 t = exp2(vec2(-l, -l - 1.0));

    const vec2 px = 1.0 - t.x + c * t.y;
    const vec2 py = 1.0 - t.y + c * t.y;

    return vec3(texture2D(index, vec2(px.s, py.t)).r,
                texture2D(index, vec2(px.s, px.t)).r,
                texture2D(index, vec2(py.s, px.t)).r);
}

void main()
{
    // Determine the coordinate of this pixel.

    const float pi = 3.14159265358979323846;

    const vec2 ck = vec2(0.5, 1.0) / pi;
    const vec2 cd = vec2(0.5, 0.5);

    vec2 c = texture2DRect(cyl, gl_FragCoord.xy).xy * ck + cd;

    // Determine the mipmap levels.

    float ll = miplev(c * data_size);
    float l0 = floor(ll);
    float l1 = ceil (ll);

    float ld = ll - l0;

    // Look up the two pages in the index.

    vec3 C0 = mipref(c, l0);
    vec3 C1 = mipref(c, l1);

    // Compute the cache coordinates for these pages.

    vec2 p0 = (data_size * c) / (page_size * exp2(C0.z * 255.0));
    vec2 p1 = (data_size * c) / (page_size * exp2(C1.z * 255.0));

    vec2 q0 = p0 + C0.xy * page_size;
    vec2 q1 = p1 + C1.xy * page_size;

    // Reference the cache and write the color.

    vec4 D0 = texture2D(cache, q0);
    vec4 D1 = texture2D(cache, q1);

//  gl_FragColor = mix(D0, D1, ld);
//  gl_FragColor = vec4(c, 0.0, 1.0);
    gl_FragColor = vec4(mix(C0, C1, ld), 1.0);
//  gl_FragColor = vec4(C1, 1.0);
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

    gl_FragColor = vec4(texture2D(index, t).rgb, K);
}
*/
