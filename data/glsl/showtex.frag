#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect src;

uniform vec2 size;

void main()
{
    vec2 t = gl_FragCoord.xy;
    vec2 a = step(vec2(0.0), t) * step(t, size);

    float K = a.x * a.y;

    gl_FragColor = vec4(texture2DRect(src, t).rgb, K);
}
