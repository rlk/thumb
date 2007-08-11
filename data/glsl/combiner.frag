uniform samplerRect L_map;
uniform samplerRect R_map;
uniform float       cycle;
uniform float       quality;

varying vec3 L_phase;
varying vec3 R_phase;

void main()
{
    const vec4 L = textureRect(L_map, gl_FragCoord.xy * quality);
    const vec4 R = textureRect(R_map, gl_FragCoord.xy * quality);

    vec3 Lk = step(vec3(cycle), fract(L_phase));
    vec3 Rk = step(vec3(cycle), fract(R_phase));

    gl_FragColor = vec4(max(L.rgb * Lk, R.rgb * Rk), 1.0);
}
