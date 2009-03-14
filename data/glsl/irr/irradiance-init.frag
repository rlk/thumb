uniform samplerCube L;
uniform samplerCube d;
uniform samplerCube Y;

uniform vec3 test;

void main()
{
    float s = gl_TexCoord[0].s * 2.0 - 1.0;
    float t = gl_TexCoord[0].t * 2.0 - 1.0;

    vec3 x = vec3(1.0, t, s);
    vec3 y = vec3(t, 1.0, s);
    vec3 z = vec3(t, s, 1.0);
/*
    gl_FragColor =
        vec4(vec3(textureCube(d,  x).r +
                  textureCube(d, -x).r +
                  textureCube(d,  y).r +
                  textureCube(d, -y).r +
                  textureCube(d,  z).r +
                  textureCube(d, -z).r), 1.0);
*/
/*
    gl_FragColor =
        (textureCube(L,  x) * textureCube(Y,  x).r +
         textureCube(L, -x) * textureCube(Y, -x).r +
         textureCube(L,  y) * textureCube(Y,  y).r +
         textureCube(L, -y) * textureCube(Y, -y).r +
         textureCube(L,  z) * textureCube(Y,  z).r +
         textureCube(L, -z) * textureCube(Y, -z).r) / 6.0;
*/

    gl_FragColor =
        (textureCube(L,  x) * textureCube(d,  x).r * textureCube(Y,  x).r +
         textureCube(L, -x) * textureCube(d, -x).r * textureCube(Y, -x).r +
         textureCube(L,  y) * textureCube(d,  y).r * textureCube(Y,  y).r +
         textureCube(L, -y) * textureCube(d, -y).r * textureCube(Y, -y).r +
         textureCube(L,  z) * textureCube(d,  z).r * textureCube(Y,  z).r +
         textureCube(L, -z) * textureCube(d, -z).r * textureCube(Y, -z).r);

//  gl_FragColor = vec4(test, 1.0);
}

