uniform sampler2D glow;
uniform sampler2D fill;
uniform sampler2D normal;

varying vec3  P;
varying vec3 fV;
varying vec3 fL;

uniform float time;

// Return the normal of the water.

vec3 norm(vec3 V, vec3 L)
{
    const vec3 Y = vec3(0.0, 1.0, 0.0);

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

    // Fade that vector to vertical toward the horizon and normalize.

    return normalize(mix(Y, N, pow(-V.y, 0.5)));
}

// Return the color of the face of the sun.

vec3 sun(vec3 V, vec3 L)
{
    return vec3(smoothstep(0.999, 0.9995, dot(V, L)));
}

// Return the color of the sky.

vec3 sky(vec3 V, vec3 L)
{
    float V_L = dot(V, L);

    return (texture2D(glow, vec2((L.y + 1.0) / 2.0, V_L)).rgb
          + texture2D(fill, vec2((L.y + 1.0) / 2.0, V.y)).rgb);
}

// Compute the color of the water.

vec3 water(vec3 V, vec3 L, vec3 N)
{
    // Calculate an ocean color for the current sun angle.

    vec3 Ko = vec3(0.03, 0.30, 0.30) * pow(max(0.0, L.y), 0.25);

    // Calculate a Fresnel coefficient for the current view.

    float f = clamp(pow(1.0 - dot(V, N), 3.0), 0.0, 1.0);

    // Mix the ocean color with the reflection of the sky.

    return mix(Ko, sky(V, L), f);
}

void main()
{
    vec3 V = normalize(fV);
    vec3 L = normalize(fL);

    vec3 N = norm(V, L);
    vec3 W = reflect(V, N);

    float d = smoothstep(-0.01, 0.0, V.y);

    gl_FragColor = vec4(mix(sun(W, L) + water(W, L, N),
                            sun(V, L) +   sky(V, L), d), 1.0);
}
