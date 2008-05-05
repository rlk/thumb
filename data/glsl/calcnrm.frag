#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect src;
uniform sampler2DRect lut;
uniform float         siz;

void main()
{
    vec2 t  = texture2DRect(lut, vec2(gl_FragCoord.x, 0.5)).xw;
    vec2 k  = t * 65535.0;
    vec3 n0 = texture2DRect(src, vec2(k.x, gl_FragCoord.y)).xyz;
    vec3 n1 = texture2DRect(src, vec2(k.y, gl_FragCoord.y)).xyz;

    gl_FragColor = vec4(normalize(n0 + n1), acos(dot(n0, n1)));
//  gl_FragColor = vec4((n0 + n1) * 0.5, acos(dot(n0, n1)));
}
