#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect src;
uniform sampler2DRect lut;
uniform sampler2DRect nrm;
uniform sampler1D     rad;
uniform float         siz;

void main()
{
    const float pi = 3.14159265358979323844;

    vec4 N  = texture2DRect(nrm, gl_FragCoord.xy);

    vec2 k  = texture2DRect(lut, vec2(gl_FragCoord.x, 0.5)).xw * 65535.0;
    vec3 p0 = texture2DRect(src, vec2(k.x, gl_FragCoord.y)).xyz;
    vec3 p1 = texture2DRect(src, vec2(k.y, gl_FragCoord.y)).xyz;

    float scl = (siz - 1.0) / siz;   // TODO: factor out
    float off = 1.0 / (2.0 * siz);   // TODO: factor out

    float r = texture1D(rad, off + scl * N.w / pi).r;

    vec3 p = 0.5 * (p0 + p1);
    vec3 d = 0.5 * (p0 - p1);

    gl_FragColor = vec4(p + N.xyz * length(d) * r, 0.0);
}
