
uniform sampler2D diffuse;

void main()
{
    vec4 Kd = texture2D(diffuse, gl_TexCoord[0].xy);

    gl_FragColor = Kd * gl_LightModel.ambient + gl_FrontMaterial.emission;
}
