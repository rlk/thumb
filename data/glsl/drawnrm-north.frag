#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect cyl;
uniform sampler2D     src;

void main()
{
    vec4 c = texture2DRect(cyl, gl_FragCoord.xy);
    vec2 t = vec2((c.z + 1.0) * 0.5, (-c.w + 1.0) * 0.5);

    vec2  a = step(vec2(0.0), t) * step(t, vec2(1.0));
    float k = a.x * a.y;

    vec3 N = vec3(c.z, sin(c.y), c.w);
    mat3 T;

    const vec3 Y = vec3(0.0, 1.0, 0.0);

    T[0] = normalize(cross(Y, N   ));
    T[1] = normalize(cross(N, T[0]));
    T[2] = N;

    vec3 n = T * (normalize(texture2D(src, t).xyz) * 2.0 - 1.0);

    gl_FragColor = vec4((n + 1.0) * 0.5, k);
//  gl_FragColor = vec4((N + 1.0) * 0.5, k);
}
