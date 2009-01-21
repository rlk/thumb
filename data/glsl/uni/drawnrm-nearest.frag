#extension GL_ARB_texture_rectangle : enable

//-----------------------------------------------------------------------------

uniform sampler2DRect cyl;
uniform vec2          cylk;
uniform vec2          cyld;

#include "glsl/uni/mipmap-common.frag"
#include "glsl/uni/mipmapL.frag"

//-----------------------------------------------------------------------------

void main()
{
    // Determine the coordinate of this pixel.

    vec4 C = texture2DRect(cyl, gl_FragCoord.xy);
    vec2 c = C.xy * cylk + cyld;

    vec2 b = step(vec2(0.0), c) * step(c, vec2(1.0));

    // Sample the normal.

    vec4 t = cacherefL(c, floor(miplevL(c * data_size)));

//  if (t.a < 1.0) discard;
    float a = t.a * b.x * b.y;

    vec3 n = t.xyz * 2.0 - 1.0;

    // Transform it into tangent space and write it.  TODO: in drawlit?

    vec3 N = vec3(C.z, sin(C.y), C.w);
    mat3 T;

    const vec3 Y = vec3(0.0, 1.0, 0.0);

    T[0] = normalize(cross(Y, N   ));
    T[1] = normalize(cross(N, T[0]));
    T[2] = N;

    gl_FragColor = vec4((T * n + 1.0) * 0.5, a);
}

//-----------------------------------------------------------------------------
