#version 120

uniform sampler2DRect Image;
uniform vec2          ImageSize;
uniform vec2          LensCenter;
uniform vec4          DistortionK;
uniform vec4          ChromaAbCorrection;
uniform vec2          ScaleIn;
uniform vec2          ScaleOut;

vec2 GetOut(vec2 v)
{
    return LensCenter + ScaleOut * v;
}

void main()
{
    vec2  v  = ((gl_TexCoord[0].xy + 1.0) * 0.5 - LensCenter) * ScaleIn;
    float rr = v.x * v.x + v.y * v.y;

    vec2 w = v * (DistortionK.x +
                  DistortionK.y * rr +
                  DistortionK.z * rr * rr +
                  DistortionK.w * rr * rr * rr);

    vec2 tb = GetOut(w * (ChromaAbCorrection.z + ChromaAbCorrection.w * rr));
    vec2 tr = GetOut(w * (ChromaAbCorrection.x + ChromaAbCorrection.y * rr));
    vec2 tg = GetOut(w);

    vec2 b = step(vec2(0.0), tb) * step(tb, vec2(1.0));

    gl_FragColor = b.x * b.y * vec4(texture2DRect(Image, ImageSize * tr).r,
                                    texture2DRect(Image, ImageSize * tg).g,
                                    texture2DRect(Image, ImageSize * tb).b, 1.0);
}
