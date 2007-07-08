#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect src;
uniform vec2          siz;

void main()
{
    vec4 E1 = texture2DRect(src, gl_FragCoord.xy      );
    vec4 E2 = texture2DRect(src, gl_FragCoord.xy + siz);

    vec4 A = min(E1, E2);
    vec4 Z = max(E1, E2);

    if (A.x <= 0.0) A.x = Z.x;

    gl_FragColor = vec4(A.x, Z.y, A.z, Z.w);
}
