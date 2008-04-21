#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect src;
uniform vec2          off;

void main()
{
    vec4 E1 = texture2DRect(src, gl_FragCoord.xy      );
    vec4 E2 = texture2DRect(src, gl_FragCoord.xy + off);

    gl_FragColor = vec4(min(E1.x, E2.x), max(E1.y, E2.y), 0.0, 1.0);
}
