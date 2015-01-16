#extension GL_ARB_texture_rectangle : enable

uniform sampler2D image[4];
uniform mat4      P[4];

float blend(vec2 t, vec2 s)
{
    vec2 e0 =     vec2(0.0, 0.0);
    vec2 e1 = s / vec2(8.0, 4.0);

    vec2 k = (1.0 - smoothstep(s - e1, s, t)) * smoothstep(e0, e1, t);

    return k.x * k.y;
}

void main()
{
    // This value gives the desired field-of-view of the dome projector.
    // The default fulldome host configuration is optimized for a 180 degree
    // FOV. Anything more will be clipped and anything less will show image
    // quality degradation. But it works in a pinch.

    const float FOV = 180.0;

    // Compute the spherical coordinate of this pixel.

    float r = length(gl_TexCoord[0].xy);
    float a =   atan(gl_TexCoord[0].y,
                     gl_TexCoord[0].x);

    // Compute the eye-space vector of this pixel.

    const float pi = 3.1415927;

    vec4 v = vec4(cos(a) * sin(r * FOV * pi / 360.0),
                           cos(r * FOV * pi / 360.0),
                  sin(a) * sin(r * FOV * pi / 360.0),
                                                1.0);

    // Project the vector onto each all off-screen framebuffer.

    vec4 v0 = P[0] * v;
    vec4 v1 = P[1] * v;
    vec4 v2 = P[2] * v;
    vec4 v3 = P[3] * v;

    // Clip the these vectors onto all off-screen frusta.

    vec4 xx = vec4(v0.x, v1.x, v2.x, v3.x);
    vec4 yy = vec4(v0.y, v1.y, v2.y, v3.y);
    vec4 zz = vec4(v0.z, v1.z, v2.z, v3.z);
    vec4 ww = vec4(v0.w, v1.w, v2.w, v3.w);

    vec4 b = step(xx, ww) * step(-ww, xx)
           * step(yy, ww) * step(-ww, xx)
           * step(zz, ww) * step(-ww, xx);

    b = vec4(1.0, 1.0, 1.0, 1.0);

    // Compute texture coordinates for each off-screen framebuffer.

    vec2 t0 = (v0.xy / v0.w + 1.0) / 2.0;
    vec2 t1 = (v1.xy / v1.w + 1.0) / 2.0;
    vec2 t2 = (v2.xy / v2.w + 1.0) / 2.0;
    vec2 t3 = (v3.xy / v3.w + 1.0) / 2.0;

    // Sample and mix all framebuffers.

    vec4 c0 = vec4(texture2D(image[0], t0).rgb, b.x * blend(t0, 1.0));
    vec4 c1 = vec4(texture2D(image[1], t1).rgb, b.y * blend(t1, 1.0));
    vec4 c2 = vec4(texture2D(image[2], t2).rgb, b.z * blend(t2, 1.0));
    vec4 c3 = vec4(texture2D(image[3], t3).rgb, b.w * blend(t3, 1.0));

    vec3 cc = (c0.rgb * c0.a +
               c1.rgb * c1.a +
               c2.rgb * c2.a +
               c3.rgb * c3.a) / (c0.a + c1.a + c2.a + c3.a);

    // This debug line demonstrates the overlap and blending.

    // cc = vec3(c0.a + c1.a + c2.a + c3.a) / 4.0;

    // Draw, clipped to the hemisphere.

    gl_FragColor = vec4(cc * step(r, 1.0), 1.0);
}
