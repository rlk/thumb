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

    gl_FragColor = cacherefL(c, floor(miplevL(c * data_size)));
}

//-----------------------------------------------------------------------------
