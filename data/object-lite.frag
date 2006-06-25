
uniform sampler2D       diffuse;
uniform sampler2DShadow shadowmap;
uniform sampler2D       lightmask;

varying vec4 vertex;
varying vec3 normal;

void main()
{
    // Look up the diffuse color and shadow map.

    vec4  Kd = texture2D    (diffuse,    gl_TexCoord[0].xy);
    float Sk = shadow2DProj (shadowmap,  gl_TexCoord[1]).r;
    vec4  Lk = texture2DProj(lightmask,  gl_TexCoord[1]);

    // Clamp the range of the light source.

    float c = (step(0.0, gl_TexCoord[1].z) *
               step(0.0, gl_TexCoord[1].w));
    c = 1.0;

    // Compute the lighting vectors.

    vec3 N = normalize(normal);
    vec3 L = normalize(gl_LightSource[0].position.xyz - vertex.xyz);

    vec3 d = vec3(max(dot(N, L), 0.0)) * Lk.rgb * Sk * c;

    gl_FrontMaterial.diffuse;

    gl_FragColor = vec4(Kd.rgb * (gl_LightSource[0].diffuse.rgb * d), Kd.a);
}
