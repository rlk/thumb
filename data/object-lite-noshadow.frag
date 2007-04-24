
uniform sampler2D diffuse;
uniform sampler2D lightmask;

varying vec4 vertex;
varying vec3 normal;

void main()
{
    // Look up the diffuse color and shadow map.

    vec4  Kd = texture2D    (diffuse,    gl_TexCoord[0].xy);
    vec4  Lk = texture2DProj(lightmask,  gl_TexCoord[1]);

    Kd = Kd * gl_FrontMaterial.diffuse;

    // Clamp the range of the light source.

    float s = gl_TexCoord[1].x / gl_TexCoord[1].w;
    float t = gl_TexCoord[1].y / gl_TexCoord[1].w;

    float c = step(0.0, gl_TexCoord[1].z) * step(0.0, s) * step(s, 1.0)
                                          * step(0.0, t) * step(t, 1.0);

    // Compute the lighting vectors.

    vec3 N = normalize(normal);
    vec3 L = normalize(gl_LightSource[0].position.xyz - vertex.xyz);

    vec3 d = vec3(max(dot(N, L), 0.0)) * Lk.rgb * c;

    gl_FragColor = vec4(Kd.rgb * (gl_LightSource[0].diffuse.rgb * d), Kd.a);
}
