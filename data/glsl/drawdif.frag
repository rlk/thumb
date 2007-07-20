#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect cyl;
uniform sampler2D     color;

void main()
{
    vec4 c = vec4(texture2DRect(cyl, gl_FragCoord.xy).xy, 0.0, 1.0);
    vec2 t = (gl_TextureMatrix[0] * c).xy;

    vec2 a = step(vec2(0.0), t) * step(t, vec2(1.0));

    gl_FragColor = vec4(texture2D(color, t).rgb, a.x * a.y);


//    gl_FragColor = texture2D(color, t);
}
