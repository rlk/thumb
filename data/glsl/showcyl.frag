#extension GL_ARB_texture_rectangle : enable

uniform sampler2D color;
//uniform mat4      clip;
varying vec4      pos;

void main()
{
    vec3 c = texture2D(color, gl_TexCoord[0].zw).rgb;
/*
    vec4 d = clip * pos;

    if (any(lessThan(d, vec4(0.0))))
        discard;
*/
    gl_FragColor = vec4(c, gl_FrontMaterial.diffuse.a);
}
