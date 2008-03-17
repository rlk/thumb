#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect cyl;
uniform sampler2D     color;

uniform vec2 d;
uniform vec2 k;

void main()
{
    vec2 t = texture2DRect(cyl, gl_FragCoord.xy).xy * k + d;

    vec2  a = step(vec2(0.0), t) * step(t, vec2(1.0));
    float k = a.x * a.y;

    gl_FragColor = vec4(texture2D(color, t).rgb, 1.0); //k
}
