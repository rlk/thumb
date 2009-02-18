
uniform sampler2D glow;
uniform sampler2D fill;

varying vec3 V_v;
varying vec3 L_v;

void main()
{
    vec3 V = normalize(V_v);
    vec3 L = normalize(L_v);

    // Look up the sky fill and glow colors.

    vec4 Kg = texture2D(glow, vec2((L.y + 1.0) / 2.0, dot(V, L)));
    vec4 Kf = texture2D(fill, vec2((L.y + 1.0) / 2.0, V.y));

    gl_FragColor = vec4(Kf.rgb + Kg.rgb, Kf.a);
//  gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
}
