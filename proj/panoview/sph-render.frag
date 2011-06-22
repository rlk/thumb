
uniform float size;
uniform float depth;
uniform float level;

float sat(float k)
{
    return clamp(k, 0.0, 1.0);
}

vec3 color(float k)
{
    return vec3(sat(k - 4.0) + sat(2.0 - k),
                sat(k      ) * sat(4.0 - k),
                sat(k - 2.0) * sat(6.0 - k));
}

void main()
{
    vec2  w = fwidth(gl_TexCoord[0].xy * size);
    float r = max(w.x, w.y);
    float l = log2(r);
    float k = clamp(level - l, 0.0, level);

    vec3 c = color(l);

    gl_FragColor = vec4(c, 1.0);
}