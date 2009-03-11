uniform samplerCube L;
uniform samplerCube d;
uniform samplerCube Y;

void main()
{
    float s = gl_TexCoord[0].s * 2.0 - 1.0;
    float t = gl_TexCoord[0].t * 2.0 - 1.0;

    vec3 x = vec3(1.0, t, s);
    vec3 y = vec3(t, 1.0, s);
    vec3 z = vec3(t, s, 1.0);

    gl_FragColor =
        (textureCube(L,  x) * textureCube(d,  x).r * textureCube(Y,  x).r +
         textureCube(L, -x) * textureCube(d, -x).r * textureCube(Y, -x).r +
         textureCube(L,  y) * textureCube(d,  y).r * textureCube(Y,  y).r +
         textureCube(L, -y) * textureCube(d, -y).r * textureCube(Y, -y).r +
         textureCube(L,  z) * textureCube(d,  z).r * textureCube(Y,  z).r +
         textureCube(L, -z) * textureCube(d, -z).r * textureCube(Y, -z).r);
}

