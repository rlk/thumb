#extension GL_ARB_texture_rectangle : enable

//-----------------------------------------------------------------------------

uniform sampler2DRect cyl;
uniform vec2          cylk;
uniform vec2          cyld;

#include "glsl/mipmap-common.frag"
#include "glsl/mipmap0.frag"

//-----------------------------------------------------------------------------

void main()
{
    // Determine the coordinate of this pixel.

    vec4 C = texture2DRect(cyl, gl_FragCoord.xy);
    vec2 c = C.xy * cylk + cyld;

    // Sample the normal.

    vec3 n = cacheref0(c).xyz * 2.0 - 1.0;

    // Transform it into tangent space and write it.  TODO: in drawlit?

    vec3 N = vec3(C.z, sin(C.y), C.w);
    mat3 T;

    const vec3 Y = vec3(0.0, 1.0, 0.0);

    T[0] = normalize(cross(Y, N   ));
    T[1] = normalize(cross(N, T[0]));
    T[2] = N;

    gl_FragColor = vec4((T * n + 1.0) * 0.5, 1.0);
}

//-----------------------------------------------------------------------------
