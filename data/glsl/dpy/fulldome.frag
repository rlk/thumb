#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect image[4];
uniform vec2           size[4];
uniform mat4           P[4];

void main()
{
    const float pi = 3.1415927;

    float r = length(gl_TexCoord[0].xy);
    float a =   atan(gl_TexCoord[0].y,
                     gl_TexCoord[0].x);

    vec4 v = vec4(cos(a) * sin(r * pi / 2.0),
                           cos(r * pi / 2.0),
                  sin(a) * sin(r * pi / 2.0),
                                        1.0);

    vec4 t0 = P[0] * v;
    vec4 t1 = P[1] * v;
    vec4 t2 = P[2] * v;
    vec4 t3 = P[3] * v;

    vec4 xx = vec4(t0.x, t1.x, t2.x, t3.x);
    vec4 yy = vec4(t0.y, t1.y, t2.y, t3.y);
    vec4 zz = vec4(t0.z, t1.z, t2.z, t3.z);
    vec4 ww = vec4(t0.w, t1.w, t2.w, t3.w);

    float rr = step(r, 1.0);

    vec4 b = step(xx, ww) * step(-ww, xx)
           * step(yy, ww) * step(-ww, xx)
           * step(zz, ww) * step(-ww, xx);

    vec3 c0 = b.x * texture2DRect(image[0], size[0] * (t0.xy / t0.w + 1.0) / 2.0).rgb;
    vec3 c1 = b.y * texture2DRect(image[1], size[1] * (t1.xy / t1.w + 1.0) / 2.0).rgb;
    vec3 c2 = b.z * texture2DRect(image[2], size[2] * (t2.xy / t2.w + 1.0) / 2.0).rgb;
    vec3 c3 = b.w * texture2DRect(image[3], size[3] * (t3.xy / t3.w + 1.0) / 2.0).rgb;

    vec3 cc = max(max(c0, c1), max(c2, c3));
    vec3 bb = vec3(b.x, b.y, b.z);

    gl_FragColor = vec4(cc * rr, 1.0);
}
