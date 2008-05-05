#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect pos;
uniform sampler2DRect nrm;
uniform sampler2DRect tex;

uniform vec2 cylk;
uniform vec2 cyld;

#include "glsl/mipmap-common.frag"
#include "glsl/mipmap0.frag"

//-----------------------------------------------------------------------------

void main()
{
    // Determine the coordinate of this pixel.

    vec2 c = texture2DRect(tex, gl_FragCoord.xy).xy * cylk + cyld;

    c.x -= step(1.0, c.x);

//  c = floor(c * data_size + 0.5) / data_size;

    // Reference the cache.

    vec4 s = cacheref0(c);

    if (s.a < 1.0) discard;

    // Compute the position.

    const float off =     0.5;
    const float mag = 65535.0;

    vec3 P = texture2DRect(pos, gl_FragCoord.xy).xyz;
    vec3 N = texture2DRect(nrm, gl_FragCoord.xy).xyz;

    float M = (s.r - off) * mag;

    gl_FragColor = vec4(P + N * M, M);
}

//-----------------------------------------------------------------------------
