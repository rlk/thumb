#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect src;
uniform vec2          size;

void main()
{
    vec2 t0 = 2.0 * gl_FragCoord.xy + vec2(-0.5, -0.5);
    vec2 t1 = 2.0 * gl_FragCoord.xy + vec2(-0.5, +0.5);
    vec2 t2 = 2.0 * gl_FragCoord.xy + vec2(+0.5, -0.5);
    vec2 t3 = 2.0 * gl_FragCoord.xy + vec2(+0.5, +0.5);

    // This test only needs to be done on 1 2 3?

    vec2 k0 = step(size, t0);
    vec2 k1 = step(size, t1);
    vec2 k2 = step(size, t2);
    vec2 k3 = step(size, t3);

    vec4 b;

    b.x = 1.0 - k0.x * k0.y;
    b.y = 1.0 - k1.x * k1.y;
    b.z = 1.0 - k2.x * k2.y;
    b.w = 1.0 - k3.x * k3.y;

    b = b / dot(b, vec4(1.0));

    mat4 C = mat4(texture2DRect(src, t0),
                  texture2DRect(src, t1),
                  texture2DRect(src, t2),
                  texture2DRect(src, t3));

    gl_FragColor = C * b;
}
