#extension GL_ARB_texture_rectangle : enable

//-----------------------------------------------------------------------------

uniform sampler2DRect cyl;
//uniform sampler1D     rp2;
uniform sampler2D     index; // 1
uniform sampler2D     cache; // 2

uniform vec2 data_size;
/*
uniform vec2 tile_size;
uniform vec2 base_size;
uniform vec2 page_size;
uniform vec2 pool_size;
*/

uniform vec2 data_over_tile;
uniform vec2 data_over_tile_base;
uniform vec2 tile_over_pool;
uniform vec2 page_over_pool;
uniform vec2 step_over_pool;

uniform vec2 cylk;
uniform vec2 cyld;

//-----------------------------------------------------------------------------

float miplev(vec2 p)
{
    vec2  w   = fwidth(p);
    float rho = max(w.x, w.y);

    return clamp(log2(rho), 0.0, 7.0);
}

//-----------------------------------------------------------------------------

vec3 mipref(vec2 c, float l)
{
    const vec2 t = exp2(vec2(-l, -l - 1.0));

    const vec2 p0 = 1.0 - t.x + c * t.y;
    const vec2 p1 = 1.0 - t.y + c * t.y;

    return vec3(texture2D(index, vec2(p0.s, p1.t)).r,
                texture2D(index, vec2(p0.s, p0.t)).r,
                texture2D(index, vec2(p1.s, p0.t)).r);
}

vec3 mipref0(vec2 c)
{
    const vec2 p0 = c * 0.5;
    const vec2 p1 = c * 0.5 + 0.5;

    return vec3(texture2D(index, vec2(p0.s, p1.t)).r,
                texture2D(index, vec2(p0.s, p0.t)).r,
                texture2D(index, vec2(p1.s, p0.t)).r);
}

//-----------------------------------------------------------------------------

vec4 cacheref(vec2 c, float l)
{
    // Look up the two pages in the index.

    vec3 C = mipref(data_over_tile_base * c, l) * 255.0;

    // Compute cache coordinates for these pages.

    vec2 p = data_over_tile * c / C.z;

    vec2 q = (tile_over_pool * fract(p) +
              page_over_pool * C.xy     +
              step_over_pool);

    // Return the color in the cache.

    return texture2D(cache, q);
}

vec4 cacheref0(vec2 c)
{
    // Look up the two pages in the index.

    vec3 C = mipref0(data_over_tile_base * c) * 255.0;

    // Compute cache coordinates for these pages.

    vec2 p = data_over_tile * c / C.z;

    vec2 q = (tile_over_pool * fract(p) +
              page_over_pool * C.xy     +
              step_over_pool);

    // Return the color in the cache.

    return texture2D(cache, q);
}

//-----------------------------------------------------------------------------

void mainl()
{
    // Determine the coordinate of this pixel.

    vec2 c = texture2DRect(cyl, gl_FragCoord.xy).xy * cylk + cyld;

    // Determine the mipmap levels.

    float ll = miplev(c * data_size);
    float l0 = floor(ll);
    float l1 = l0 + 1.0;

    float ld = ll - l0;

    gl_FragColor = mix(cacheref(c, l0),
                       cacheref(c, l1), ld);
}

void main()
{
    // Determine the coordinate of this pixel.

    vec2 c = texture2DRect(cyl, gl_FragCoord.xy).xy * cylk + cyld;

    gl_FragColor = cacheref0(c);
}

//-----------------------------------------------------------------------------
