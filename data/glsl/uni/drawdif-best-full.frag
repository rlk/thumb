#extension GL_ARB_texture_rectangle : enable

//-----------------------------------------------------------------------------

uniform sampler2DRect cyl;
uniform vec2          cylk;
uniform vec2          cyld;

#include "glsl/uni/mipmap-common.frag"
#include "glsl/uni/mipmap0.frag"

//-----------------------------------------------------------------------------

void main()
{
    // Determine the coordinate of this pixel.

    vec2 c = texture2DRect(cyl, gl_FragCoord.xy).xy * cylk + cyld;

    gl_FragColor = cacheref0(c);
}

//-----------------------------------------------------------------------------