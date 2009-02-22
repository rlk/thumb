#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect src;
uniform sampler2DRect avg;
uniform sampler2DRect bloom;

void main()
{
    const vec2 d = vec2(0.5);

    vec4 a = texture2DRect(avg,   d);
    vec4 b = texture2DRect(bloom, d * gl_FragCoord.xy);
    vec4 c = texture2DRect(src,       gl_FragCoord.xy);

    gl_FragColor = b + c;
}
