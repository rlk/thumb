uniform samplerCube L;
uniform samplerCube d;
uniform samplerCube Y;

uniform vec2 loc;
uniform vec2 siz;

void main()
{
    gl_FragColor = vec4(0.5 * loc / siz, 0.0, 1.0);
//  gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
}

