//varying vec3 normal;

void main()
{
    const float pi = 3.14159265358979323844;

//  vec3 N = normalize(normal);
    vec2 c = gl_TexCoord[0].xy;
    vec2 n = gl_TexCoord[0].zw;

    // Wrap cylindrical coordinates at pi.

    c.x -= 2.0 * pi * step(pi, c.x);

    gl_FragColor = vec4(c, n); // TODO normalize n.
//  gl_FragColor = vec4(c, 0, 1); // TODO normalize n.
}
