uniform sampler2D glow;
uniform sampler2D fill;
uniform sampler2D normal;

varying vec3  P;
varying vec3 fV;
varying vec3 fL;

uniform float time;

void main()
{
    vec3 V = normalize(fV);
    vec3 L = normalize(fL);

    // Compute water texture coordinates.

    vec2 t0 = -P.xz + V.xz * P.y / V.y;

    vec2 t1 = 0.0005 * t0 + vec2(-0.003,  0.005) * time;
    vec2 t2 = 0.0050 * t0 + vec2(-0.010, -0.020) * time;
    vec2 t3 = 0.0220 * t0 + vec2( 0.014,  0.030) * time;
    vec2 t4 = 0.0510 * t0 + vec2( 0.040, -0.050) * time;

    // The water normal is the sum of multiple normal map references.

    const vec3 ck = vec3(-2.0, +2.0, -2.0);
    const vec3 cd = vec3(+1.0, -1.0, +1.0);

    vec3 N = normalize((texture2D(normal, t1).xzy * ck + cd) +
                       (texture2D(normal, t2).xzy * ck + cd) +
                       (texture2D(normal, t3).xzy * ck + cd) +
                       (texture2D(normal, t4).xzy * ck + cd));

    // Fade the normal to vertical toward the horizon.

    N = mix(vec3(0.0, 1.0, 0.0), N, -V.y);

    // Reflect a downward view vector across the water.

    float dn = step(V.y, 0.0);

    V = mix(V, reflect(V, N), dn);

    // Look up the sky fill and glow colors.

    vec4 Tg = texture2D(glow, vec2((L.y + 1.0) / 2.0, dot(V, L)));
    vec4 Tf = texture2D(fill, vec2((L.y + 1.0) / 2.0, V.y));

    // Mix the ocean color using a Fresnel coefficient.

    float f = mix(1.0, clamp(pow(1.0 - dot(N, V), 3.0), 0.0, 1.0), dn);

    vec3 Ko = vec3(0.03, 0.15, 0.15) * max(0.0, pow(L.y, 0.25));

    vec4 K = mix(vec4(Ko, 1.0), Tf + Tg, f);

    gl_FragColor = vec4(K.rgb, Tf.a);
}
