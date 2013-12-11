
uniform sampler2D glow;
uniform sampler2D fill;
uniform sampler2D norm;

varying vec3 V_v;

uniform float time;
uniform vec3  view_position;
uniform vec4  light_position;

void main()
{
    vec3 V = normalize(V_v);
    vec3 L = normalize(light_position.xyz);

    // Compute water texture coordinates.

    float h =  view_position.y;
    vec2 t0 = -view_position.xz + V.xz * view_position.y / V.y;
/*
    vec2 t0 = V.xz * 5.0 / V.y;
*/

    vec2 t1 = 0.0005 * t0 + vec2(-0.003,  0.005) * time;
    vec2 t2 = 0.0050 * t0 + vec2(-0.010, -0.020) * time;
    vec2 t3 = 0.0220 * t0 + vec2( 0.014,  0.030) * time;
    vec2 t4 = 0.0510 * t0 + vec2( 0.040, -0.050) * time;

    // The water normal is the sum of multiple normal map references.

    const vec3 ck = vec3(-2.0, +2.0, -2.0);
    const vec3 cd = vec3(+1.0, -1.0, +1.0);

    vec3 N = normalize((texture2D(norm, t1).xzy * ck + cd) +
                       (texture2D(norm, t2).xzy * ck + cd) +
                       (texture2D(norm, t3).xzy * ck + cd) +
                       (texture2D(norm, t4).xzy * ck + cd));

    // Fade the normal to vertical toward the horizon.

    N = mix(vec3(0.0, 1.0, 0.0), N, -V.y);

    // Reflect a downward view vector across the water.

    float dn = step(V.y, 0.0);

    V = mix(V, reflect(V, N), dn);

    // Look up the sky fill and glow colors.

    vec4 Kg = texture2D(glow, vec2((L.y + 1.0) / 2.0, dot(V, L)));
    vec4 Kf = texture2D(fill, vec2((L.y + 1.0) / 2.0, V.y));

    float Ks = 1000.0 * pow(max(dot(L, V), 0.0), 4096.0);

    // Mix the ocean color using a Fresnel coefficient.

    float f = mix(1.0, clamp(pow(1.0 - dot(N, V), 2.0), 0.0, 1.0), dn);

    vec4 Ko = vec4(0.0, 0.0, 0.0, 1.0);

    vec4 K = mix(Ko, Kf + Kg + vec4(Ks, Ks, Ks, 1.0), f);

    gl_FragColor = vec4(K.rgb, Kf.a);
//  gl_FragColor = vec4((Kf + Kg).rgb, Kf.a);
//  gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
}
