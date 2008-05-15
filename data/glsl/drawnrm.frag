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

    vec4 C = texture2DRect(cyl, gl_FragCoord.xy);
    vec2 c = C.xy * cylk + cyld;

    // Determine the mipmap levels.

    float ll = miplevL(c * data_size);
    float l0 = floor(ll);
    float l1 = l0 + 1.0;

    // Sample the normal.  TODO: normalization may be unnecessary.

    vec4 n0 = cacherefL(c, l0) * vec4(vec3(2.0), 1.0)
                               - vec4(vec3(1.0), 0.0);
    vec4 n1 = cacherefL(c, l1) * vec4(vec3(2.0), 1.0)
                               - vec4(vec3(1.0), 0.0);
    vec4 n = mix(n0, n1, ll - l0);

    if (n.a < 1.0) discard;
/*
    vec3 n = normalize(mix(cacherefL(c, l0),
                           cacherefL(c, l1), ll - l0).xyz * 2.0 - 1.0);
*/
    // Transform it into tangent space and write it.  TODO: in drawlit?

    vec3 N = vec3(C.z, sin(C.y), C.w);
    mat3 T;

    const vec3 Y = vec3(0.0, 1.0, 0.0);

    T[0] = normalize(cross(Y, N   ));
    T[1] = normalize(cross(N, T[0]));
    T[2] = N;

    gl_FragColor = vec4((T * n.xyz + 1.0) * 0.5, 1.0);
}

//-----------------------------------------------------------------------------
