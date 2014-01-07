uniform sampler2D glow;
uniform sampler2D fill;
uniform sampler2D normal;

varying vec3  P;
varying vec3 fV;
varying vec3 fL;

vec4 earth(vec3 V, vec3 L)
{
    vec4 a = vec4(0.4, 0.3, 0.2, 1.0);
    vec4 b = vec4(1.0, 1.0, 1.0, 1.0);

    return mix(b, a, pow(abs(V.y), 0.2)) * max(0.0, L.y);
}

vec4 sky(vec3 V, vec3 L)
{
    float V_L = dot(V, L);

    vec3 Tg = texture2D(glow, vec2((L.y + 1.0) / 2.0, V_L)).rgb;
    vec3 Tf = texture2D(fill, vec2((L.y + 1.0) / 2.0, V.y)).rgb;

    vec3 Cs = vec3(smoothstep(0.999, 0.9995, V_L));

    return vec4(Tg + Tf + Cs, 1.0);
}

void main()
{
    vec3 V = normalize(fV);
    vec3 L = normalize(fL);

    gl_FragColor = mix(earth(V, L), sky(V, L), step(0.0, V.y));
}