#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect src;
uniform sampler2DRect avg;
uniform sampler2DRect bloom;

void main()
{
    const vec2 d = vec2(0.5);

    float A = dot(texture2DRect(avg, d).rgb, vec3(0.30, 0.59, 0.11));

    float L = A;

    float E = 1.0 / L;

    vec3 b = texture2DRect(bloom, d * gl_FragCoord.xy).rgb;
    vec3 c = texture2DRect(src,       gl_FragCoord.xy).rgb;

    vec3 C = 1.0 - exp(-E * c);

    gl_FragColor = vec4(C + b, 1.0);
}
