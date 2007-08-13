#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect L_map;
uniform sampler2DRect R_map;
uniform float         cycle;

varying vec3 L_phase;
varying vec3 R_phase;
uniform vec2 k;
uniform vec2 d;

void main()
{
    vec2 p = (gl_FragCoord.xy + d) * k;

    const vec4 L = texture2DRect(L_map, p);
    const vec4 R = texture2DRect(R_map, p);

    vec3 Lk = step(vec3(cycle), fract(L_phase));
    vec3 Rk = step(vec3(cycle), fract(R_phase));

    gl_FragColor = vec4(max(L.rgb * Lk, R.rgb * Rk), 1.0);
}
