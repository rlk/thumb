
uniform sampler2D glow;
uniform sampler2D fill;
uniform sampler2D norm;

varying vec3 V_v;
varying vec3 L_v;

void main()
{
    const float h = 5.0;
    const vec3  Y = vec3(0.0, 1.0, 0.0);
    
    vec3 V = normalize(V_v);
    vec3 L = normalize(L_v);

    // Reflect the view vector across the water.

//  vec3 N = normalize(2.0 * texture2D(norm, V.xz * h / V.y).xzy - 1.0);
    vec3 c = texture2D(norm, vec2(-V.x, V.z) * h / V.y).xyz;
    vec3 N = normalize(2.0 * c - 1.0);

    N = vec3(N.x, N.z, -N.y);
    N = mix(Y, N, -V.y);

    vec3 R = reflect(V, N);

    V = mix(R, V, step(0.0, V.y));

    // Look up the sky fill and glow colors.

    vec4 Kg = texture2D(glow, vec2((L.y + 1.0) / 2.0, dot(V, L)));
    vec4 Kf = texture2D(fill, vec2((L.y + 1.0) / 2.0, V.y));

    gl_FragColor = vec4(Kf.rgb + Kg.rgb, Kf.a);
//  gl_FragColor = vec4(c, 1.0);
}
