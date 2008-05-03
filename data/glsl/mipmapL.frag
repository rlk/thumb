//-----------------------------------------------------------------------------

float miplevL(vec2 p)
{
    vec2  w   = fwidth(p);
    float rho = max(w.x, w.y);

//  return clamp(log2(rho), 0.0, 7.0);
    return max(log2(rho), 0.0);
}

vec3 miprefL(vec2 c, float l)
{
    const vec2 t = exp2(vec2(-l, -l - 1.0));

    const vec2 p0 = 1.0 - t.x + c * t.y;
    const vec2 p1 = 1.0 - t.y + c * t.y;

    return vec3(texture2D(index, vec2(p0.s, p1.t)).r,
                texture2D(index, vec2(p0.s, p0.t)).r,
                texture2D(index, vec2(p1.s, p0.t)).r);
}

vec4 cacherefL(vec2 c, float l)
{
    // Look up the two pages in the index.

    vec3 C = miprefL(data_over_tile_base * c, l) * 65535.0;

    // Compute cache coordinates for these pages.

    vec2 p = data_over_tile * c / C.z;

    // TODO: optimize

    vec2 q = (tile_size * fract(p) + C.xy) * over_pool;

    // Return the color in the cache.

    return texture2D(cache, q);
}

//-----------------------------------------------------------------------------
