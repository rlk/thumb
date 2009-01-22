uniform vec3 offset;
varying vec3 L_phase;
varying vec3 R_phase;

void main()
{
    vec4 dr = vec4(offset.r, 0.0, 0.0, 0.0);
    vec4 dg = vec4(offset.g, 0.0, 0.0, 0.0);
    vec4 db = vec4(offset.b, 0.0, 0.0, 0.0);

    L_phase.r = (gl_TextureMatrix[0] * (gl_Vertex + dr)).x;
    R_phase.r = (gl_TextureMatrix[1] * (gl_Vertex + dr)).x;

    L_phase.g = (gl_TextureMatrix[0] * (gl_Vertex + dg)).x;
    R_phase.g = (gl_TextureMatrix[1] * (gl_Vertex + dg)).x;

    L_phase.b = (gl_TextureMatrix[0] * (gl_Vertex + db)).x;
    R_phase.b = (gl_TextureMatrix[1] * (gl_Vertex + db)).x;

    gl_Position = ftransform();
}

