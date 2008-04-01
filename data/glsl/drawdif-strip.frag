#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect cyl;
uniform sampler2D     src;

uniform vec2 d;
uniform vec2 k;

void main()
{
    const float ca =  0.70710678118654752440;
    const float sa = -0.70710678118654752440;

    vec4 C = texture2DRect(cyl, gl_FragCoord.xy);
    vec2 c = C.zw;

    vec3 n = normalize(vec3(C.z, sin(C.y), C.w));
    vec3 m = n;
    
    m.x = n.x * ca - n.y * sa;
    m.y = n.x * sa + n.y * ca;

    c.x = atan(m.x, m.z) * 4.0;
    c.y = asin(m.y)      * 16.0;

    vec2 t = c * k + d;

    vec2  a = step(vec2(0.0), t) * step(t, vec2(1.0));
    float K = a.x * a.y;

    vec4 D = texture2D(src, t);

    gl_FragColor = vec4(D.rgb, K * D.a);
}
