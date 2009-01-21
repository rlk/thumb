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

    if (c == clamp(c, 0.0, 1.0))
    {
        // Sample the normal.

        vec4 n = cacheref0(c) * vec4(vec3(2.0), 1.0)
                              - vec4(vec3(1.0), 0.0);

        if (n.a < 1.0) discard;

        // Transform it into tangent space and write it.  TODO: in drawlit?

        vec3 N = vec3(C.z, sin(C.y), C.w);
        mat3 T;

        const vec3 Y = vec3(0.0, 1.0, 0.0);

        T[0] = normalize(cross(Y, N   ));
        T[1] = normalize(cross(N, T[0]));
        T[2] = N;

        gl_FragColor = vec4((T * n.xyz + 1.0) * 0.5, 1.0);
    }
    else discard;
}

//-----------------------------------------------------------------------------
