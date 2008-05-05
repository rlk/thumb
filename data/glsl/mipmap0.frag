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
    // Look up the page in the index.

    vec3 C = mipref0(data_over_tile_base * c) * 65535.0;

    // Compute cache coordinates for these pages.

    vec2 p = data_over_tile * c / C.z;

    vec2 q = (tile_size * fract(p) + C.xy) * over_pool;

    // Return the color in the cache.

    return texture2D(cache, q);
/*
    vec2 k = frac(q * pool_size);

    vec4 c00 = texture2D(cache, q);
    vec4 c01 = texture2D(cache, q + vec2(0.0, over_pool.y));
    vec4 c10 = texture2D(cache, q + vec2(over_pool.x, 0.0));
    vec4 c11 = texture2D(cache, q + over_pool);

    return mix(mix(c00, c01, k.x),
               mix(c10, c11, k.x), k.y);
*/
}

//-----------------------------------------------------------------------------
