//-----------------------------------------------------------------------------

vec3 mipref0(vec2 c)
{
    const vec2 p0 = c * 0.5;
    const vec2 p1 = c * 0.5 + 0.5;

    return vec3(texture2D(index, vec2(p0.s, p1.t)).r,
                texture2D(index, vec2(p0.s, p0.t)).r,
                texture2D(index, vec2(p1.s, p0.t)).r);
}

vec4 cacheref0(vec2 c)
{
    // Look up the two pages in the index.

    vec3 C = mipref0(data_over_tile_base * c) * 65535.0;

    // Compute cache coordinates for these pages.

    vec2 p = data_over_tile * c / C.z;

    // TODO: optimize

    vec2 q = (tile_size * fract(p) + C.xy + 1.0) / pool_size;
/*
    vec2 q = (tile_over_pool * fract(p) +
              page_over_pool * C.xy     +
              step_over_pool);
*/
    // Return the color in the cache.

    return texture2D(cache, q);
}

//-----------------------------------------------------------------------------
