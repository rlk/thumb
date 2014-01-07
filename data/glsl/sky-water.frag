uniform sampler2D glow;
uniform sampler2D fill;
uniform sampler2D normal;

varying vec3  P;
varying vec3 fV;
varying vec3 fL;

uniform float time;

const vec3 Y = vec3(0.0, 1.0, 0.0);

void main()
{
    vec3 V = normalize(fV);
    vec3 L = normalize(fL);

    // Compute water texture coordinates.

    vec2 t0 = P.xz - V.xz * P.y / V.y;

    vec2 t1 = 0.0005 * t0 + vec2(-0.003,  0.005) * time;
    vec2 t2 = 0.0050 * t0 + vec2(-0.010, -0.020) * time;
    vec2 t3 = 0.0220 * t0 + vec2( 0.014,  0.030) * time;
    vec2 t4 = 0.0510 * t0 + vec2( 0.040, -0.050) * time;

    // The water normal is the sum of multiple normal map references.

    vec3 N = (texture2D(normal, t1).xyz +
              texture2D(normal, t2).xyz +
              texture2D(normal, t3).xyz +
              texture2D(normal, t4).xyz - 2.0) * 0.5;

    // Fade the normal to vertical toward the horizon.

    N = normalize(mix(Y, N, pow(-V.y, 0.5)));

    // Reflect a downward view vector across the water.

    float dn = step(V.y, 0.0);

    V = mix(V, reflect(V, N), dn);

    // Look up the sky fill and glow colors.

    float x = (L.y + 1.0) / 2.0;
    float d = dot(V, L);

    vec4 Tg = texture2D(glow, vec2(x, d));
    vec4 Tf = texture2D(fill, vec2(x, V.y));

    // Calculate the color of the face of the sun.

    vec3 Cs = vec3(smoothstep(0.999, 0.9995, d));

    // Calculate an ocean color for the current sun angle.

    vec3 Ko = vec3(0.03, 0.30, 0.30) * pow(max(0.0, L.y), 0.25);

    // Mix the ocean color using a Fresnel coefficient.

    float f = mix(1.0, clamp(pow(1.0 - dot(V, N), 3.0), 0.0, 1.0), dn);

    vec3 Co = mix(Ko, vec3(Tf + Tg), f);

    gl_FragColor = vec4(Co + Cs, Tf.a);
}
