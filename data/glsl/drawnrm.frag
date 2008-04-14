#extension GL_ARB_texture_rectangle : enable

//-----------------------------------------------------------------------------

uniform sampler2DRect cyl;
uniform sampler2D     index;
uniform sampler2D     cache;

uniform vec2 data_size;
uniform vec2 tile_size;
uniform vec2 page_size;
uniform vec2 pool_size;

//-----------------------------------------------------------------------------

float miplev(vec2 p)
{
    const vec2 dpdx = dFdx(p);
    const vec2 dpdy = dFdy(p);
/*
    float rho = length(vec2(length(dpdx),
                            length(dpdy)));
*/
    float rho = max(length(dpdx),
                    length(dpdy));

    return clamp(log2(rho), 0.0, 7.0);
}

vec3 mipref(vec2 c, float l)
{
    const vec2 t = exp2(vec2(-l, -l - 1.0));

    const vec2 p0 = 1.0 - t.x + c * t.y;
    const vec2 p1 = 1.0 - t.y + c * t.y;

    return vec3(texture2D(index, vec2(p0.s, p1.t)).r,
                texture2D(index, vec2(p0.s, p0.t)).r,
                texture2D(index, vec2(p1.s, p0.t)).r);
}

void main()
{
    // Determine the coordinate of this pixel.

    const float pi = 3.14159265358979323846;

    const vec2 ck = vec2(0.5, 1.0) / pi;
    const vec2 cd = vec2(0.5, 0.5);

    vec4 C = texture2DRect(cyl, gl_FragCoord.xy);
    vec2 c = C.xy * ck + cd;

    // Determine the mipmap levels.

    float ll = miplev(c * data_size);
    float l0 = floor(ll);
    float l1 = l0 + 1.0;

    float ld = ll - l0;

    // Compute index coordinates for these pages.

    vec2 p0 = (data_size * c) / (tile_size * exp2(l0));
    vec2 p1 = (data_size * c) / (tile_size * exp2(l1));

    vec2 s0 = vec2(256.0, 128.0) / exp2(l0);
    vec2 s1 = vec2(256.0, 128.0) / exp2(l1);

    // Look up the two pages in the index.

    vec3 C0 = mipref(p0 / s0, l0) * 255.0;
    vec3 C1 = mipref(p1 / s1, l1) * 255.0;

    // Compute cache coordinates for these pages.

    p0 = (data_size * c) / (tile_size * exp2(C0.z));
    p1 = (data_size * c) / (tile_size * exp2(C1.z));

    vec2 q0 = (fract(p0) * tile_size + 1.0 + C0.xy * page_size) / pool_size;
    vec2 q1 = (fract(p1) * tile_size + 1.0 + C1.xy * page_size) / pool_size;

    // Reference the cache and find the normal.

    vec3 D0 = texture2D(cache, q0).rgb;
    vec3 D1 = texture2D(cache, q1).rgb;

    vec3 n = normalize(mix(D0, D1, ld) * 2.0 - 1.0);

    // Tranform it into tangent space and write it.

    vec3 N = vec3(C.z, sin(C.y), C.w);
    mat3 T;

    const vec3 Y = vec3(0.0, 1.0, 0.0);

    T[0] = normalize(cross(Y, N   ));
    T[1] = normalize(cross(N, T[0]));
    T[2] = N;

    gl_FragColor = vec4((T * n + 1.0) * 0.5, 1.0);
}

//-----------------------------------------------------------------------------
/*
#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect cyl;
uniform sampler2D     src;

uniform vec2 d;
uniform vec2 k;

void main()
{
    vec4 c = texture2DRect(cyl, gl_FragCoord.xy);
    vec2 t = c.xy * k + d;

    vec2  a = step(vec2(0.0), t) * step(t, vec2(1.0));
    float K = a.x * a.y;

    vec3 N = vec3(c.z, sin(c.y), c.w);
    mat3 T;

    const vec3 Y = vec3(0.0, 1.0, 0.0);

    T[0] = normalize(cross(Y, N   ));
    T[1] = normalize(cross(N, T[0]));
    T[2] = N;

    vec3 n = T * (normalize(texture2D(src, t).xyz) * 2.0 - 1.0);

    gl_FragColor = vec4((n + 1.0) * 0.5, K);
}
*/
