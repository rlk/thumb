
uniform vec2 loc;
uniform vec2 siz;

void main()
{
    gl_TexCoord[0] = gl_MultiTexCoord0;

    gl_Position = vec4((1.0 + 2.0 * loc - siz + gl_Vertex.xy) / siz, 0.0, 1.0);
//  gl_Position = gl_Vertex;
}
