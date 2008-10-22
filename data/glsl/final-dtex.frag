#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect cyl;
uniform sampler2DRect dif;
uniform sampler2DRect nrm;

varying vec3 L;

float width(float k)
{
    return length(vec2(dFdx(k), dFdy(k)));
}

void main()
{
    vec4 C = texture2DRect(cyl, gl_FragCoord.xy);

    float x = 1.0 - width(C.x) * 1024.0;
    float y = 1.0 - width(C.y) * 1024.0;

    gl_FragColor = vec4(x, y, 0.0, 1.0);
}
