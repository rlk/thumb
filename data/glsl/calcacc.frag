#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect src;
uniform sampler2DRect pos;
uniform sampler2DRect nrm;
uniform sampler2DRect tex;

#include "glsl/mipmap-common.frag"
#include "glsl/mipmap0.frag"

//-----------------------------------------------------------------------------

void main()
{
    // Determine the coordinate of this pixel.

    const float pi = 3.14159265358979323846;

    const vec2 ck = vec2(0.5, 1.0) / pi;
    const vec2 cd = vec2(0.5, 0.5);

    // TODO: worry about international date line?

    vec2 c = texture2DRect(tex, gl_FragCoord.xy).xy * ck + cd;

    // Reference the cache.

    float s = cacheref0(c).r;

    // Compute the position.

    const float off =     0.5;
    const float mag = 65535.0;

    vec3 P = texture2DRect(pos, gl_FragCoord.xy).xyz;
    vec3 N = texture2DRect(nrm, gl_FragCoord.xy).xyz;

    float M = (s - off) * mag;

    gl_FragColor = vec4(P + N * M, M);
}

//-----------------------------------------------------------------------------
