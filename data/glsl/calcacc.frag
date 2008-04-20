#extension GL_ARB_texture_rectangle : enable

//uniform sampler2D map;
uniform sampler2DRect src;
uniform sampler2DRect pos;
uniform sampler2DRect nrm;
uniform sampler2DRect tex;

uniform sampler2D index;
uniform sampler2D cache;

uniform vec2 data_size;
uniform vec2 tile_size;
uniform vec2 page_size;
uniform vec2 pool_size;

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

void main()
{
    // Determine the coordinate of this pixel.

    const float pi = 3.14159265358979323846;

    const vec2 ck = vec2(0.5, 1.0) / pi;
    const vec2 cd = vec2(0.5, 0.5);

    // TODO: worry about international date line?

    vec2 c = texture2DRect(tex, gl_FragCoord.xy).xy * ck + cd;

    // Determine the mipmap levels.

    float l0 = 0.0;

    // Compute index coordinates for these pages.

    vec2 p0 = (data_size * c) / (tile_size * exp2(l0));

    vec2 s0 = vec2(256.0, 128.0) / exp2(l0);

    // Look up the two pages in the index.

    vec3 C0 = mipref(p0 / s0, l0) * 255.0;

    // Compute cache coordinates for these pages.

    vec2 tt = data_size / tile_size;

    p0 = tt * c / C0.z;
/*
    p0 = (data_size * c) / (tile_size * exp2(C0.z));
*/
    vec2 q0 = (fract(p0) * tile_size + 1.0 + C0.xy * page_size) / pool_size;

    // Reference the cache and write the color.

    float s = texture2D(cache, q0).r;

    const float off =     0.5;
    const float scl =     1.0;
    const float mag = 65535.0;

    vec3 P = texture2DRect(pos, gl_FragCoord.xy).xyz;
    vec3 N = texture2DRect(nrm, gl_FragCoord.xy).xyz;

    float M = (s - off) * scl * mag;

    vec4 D = vec4(P + N * M, M);

    gl_FragColor = D;
}

//-----------------------------------------------------------------------------
/*
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

    const float off =     0.5;
    const float scl =     1.0;
    const float mag = 65535.0;

//  vec4 S = texture2DRect(src, gl_FragCoord.xy);
    vec3 P = texture2DRect(pos, gl_FragCoord.xy).xyz;
    vec3 N = texture2DRect(nrm, gl_FragCoord.xy).xyz;

    float s = texture2D(map, T).r;

    float M = (s - off) * scl * mag;

    vec4 D = vec4(P + N * M, M);

    gl_FragColor = D;
}
*/
