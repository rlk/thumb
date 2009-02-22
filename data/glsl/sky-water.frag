
uniform sampler2D glow;
uniform sampler2D fill;
uniform sampler2D norm;

varying vec3 V_v;
varying vec3 L_v;

uniform float time;

void main()
{
    vec3 V = normalize(V_v);
    vec3 L = normalize(L_v);

    // Compute water texture coordinates as seen from 5 meters up.

    const float h = 5.0;
    
    vec2 t0 = V.xz * h / V.y;

    vec2 t1 = 0.0600 * t0 + vec2(-0.01, -0.02) * time;
    vec2 t2 = 0.1300 * t0 + vec2( 0.01,  0.03) * time;
    vec2 t3 = 0.2500 * t0 + vec2( 0.04, -0.05) * time;

    // The water normal is the sum of three normal map references.

    const vec3 ck = vec3(-2.0, +2.0, -2.0);
    const vec3 cd = vec3(+1.0, -1.0, +1.0);

    vec3 N = normalize(texture2D(norm, t1).xzy * ck + cd +
                       texture2D(norm, t2).xzy * ck + cd +
                       texture2D(norm, t3).xzy * ck + cd);

    // Fade the normal to vertical toward the horizon.

    N = mix(vec3(0.0, 1.0, 0.0), N, -V.y);

    // Reflect a downward view vector across the water.

    float dn = step(V.y, 0.0);
//  float up = step(0.0, V.y);

    V = mix(V, reflect(V, N), dn);

    // Look up the sky fill and glow colors.

    vec4 Kg = texture2D(glow, vec2((L.y + 1.0) / 2.0, dot(V, L)));
    vec4 Kf = texture2D(fill, vec2((L.y + 1.0) / 2.0, V.y));

    // Mix the ocean color using a Fresnel coefficient.

    float f = mix(1.0, clamp(pow(1.0 - dot(N, V), 1.2), 0.0, 1.0), dn);

    vec4 Ko = vec4(0.0, 0.5, 0.5, 1.0);

    vec4 K = mix(Ko, Kf + Kg, f);

    gl_FragColor = vec4(K.rgb, Kf.a);
//  gl_FragColor = vec4(vec3(f), 1.0);
}
