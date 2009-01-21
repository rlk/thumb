#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect pos;
uniform sampler2DRect nrm;
uniform sampler2DRect tex;

uniform vec2 cylk;
uniform vec2 cyld;

#include "glsl/uni/mipmap-common.frag"
#include "glsl/uni/mipmap0.frag"

uniform float sea;
uniform vec3  ori;

//-----------------------------------------------------------------------------

void main()
{
    // Determine the coordinate of this pixel.

    vec2 c = texture2DRect(tex, gl_FragCoord.xy).xy * cylk + cyld;

    vec2 b = step(vec2(0.0), c) * step(c, vec2(1.0));

    {
        // Reference the cache.

        vec4 s = cacheref0(c);

	float a = s.a * b.x * b.y;

        // Compute the position.

        const float off =     0.5;
        const float mag = 65535.0;

//      vec3 P = texture2DRect(pos, gl_FragCoord.xy).xyz;
        vec3 N = texture2DRect(nrm, gl_FragCoord.xy).xyz;

        float M = (s.r - off) * mag;

//      gl_FragColor = vec4(P + normalize(N) * M, M);

//      gl_FragColor = vec4(ori + N * (sea + M), M);
//      gl_FragColor = vec4(ori + N * (sea + M), a);
        gl_FragColor = vec4(ori + N * (sea), a);
    }
}

//-----------------------------------------------------------------------------
