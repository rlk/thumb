#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect cyl;
uniform sampler2D     normal;

uniform vec3 axis;

uniform vec2 coff;
uniform vec2 cscl;

void main()
{
    vec4 C = texture2DRect(cyl, gl_FragCoord.xy);

    vec2 t = (C.xy + coff) * cscl;
    vec3 N = normalize(vec3(C.zw, sqrt(1.0 - C.z * C.z - C.w * C.w)));

    mat3 T;

    T[0] = cross(axis, N);
    T[1] = cross(N, T[0]);
    T[2] = N;

    vec3 n = T * (texture2D(normal, t).xyz * 2.0 - 1.0);

    gl_FragColor = vec4((n + 1.0) * 0.5, 0.0);
}
