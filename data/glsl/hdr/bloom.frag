#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect src;
uniform sampler2DRect bloom;

void main()
{
    const vec2 d = vec2(0.5);

    vec3 b = texture2DRect(bloom, d * gl_FragCoord.xy).rgb;
    vec3 c = texture2DRect(src,       gl_FragCoord.xy).rgb;

    gl_FragColor = vec4(b + c, 1.0);
}
