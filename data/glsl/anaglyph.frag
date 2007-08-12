#extension GL_ARB_texture_rectangle : enable

uniform samplerRect L_map;
uniform samplerRect R_map;

uniform vec2 k;
uniform vec2 d;

void main()
{
    const vec4 luma = { 0.30, 0.59, 0.11, 0.00 };

    vec2 p = (gl_FragCoord.xy + d) * k;

    const vec4 L = textureRect(L_map, p);
    const vec4 R = textureRect(R_map, p);

    gl_FragColor = vec4(dot(L, luma) * 0.36, 0.0, dot(R, luma), 1.0);
}
