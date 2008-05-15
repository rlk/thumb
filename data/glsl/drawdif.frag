#extension GL_ARB_texture_rectangle : enable

//-----------------------------------------------------------------------------

uniform sampler2DRect cyl;
uniform vec2          cylk;
uniform vec2          cyld;

#include "glsl/mipmap-common.frag"
#include "glsl/mipmapL.frag"

//-----------------------------------------------------------------------------

void main()
{
    // Determine the coordinate of this pixel.

    vec2 c = texture2DRect(cyl, gl_FragCoord.xy).xy * cylk + cyld;

    c.x -= step(1.0, c.x);

    // Determine the mipmap levels.

    float ll = miplevL(c * data_size);
    float l0 = floor(ll);
    float l1 = l0 + 1.0;

    // Sample the color.

    vec4 cc = mix(cacherefL(c, l0),
                  cacherefL(c, l1), ll - l0);

    if (cc.a < 1.0) discard;

    gl_FragColor = vec4(cc.rgb, 1.0);
}

//-----------------------------------------------------------------------------
