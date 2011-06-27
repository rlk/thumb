#extension GL_EXT_geometry_shader4 : require

/*
void set(int i)
{
    gl_TexCoord[7] = gl_TexCoordIn[i][7];
    gl_TexCoord[6] = gl_TexCoordIn[i][6];
    gl_TexCoord[5] = gl_TexCoordIn[i][5];
    gl_TexCoord[4] = gl_TexCoordIn[i][4];
    gl_TexCoord[3] = gl_TexCoordIn[i][3];
    gl_TexCoord[2] = gl_TexCoordIn[i][2];
    gl_TexCoord[1] = gl_TexCoordIn[i][1];
    gl_TexCoord[0] = gl_TexCoordIn[i][0];
    gl_Position    = gl_PositionIn[i];
}

void main(void) 
{
    set(0);
    EmitVertex();

    set(1);
    EmitVertex();
    
    set(2);
    EmitVertex();

    EndPrimitive();
}
*/

void emit(vec3 v, vec2 t)
{
    gl_Position    = gl_ModelViewProjectionMatrix * vec4(v, 1.0);
    gl_TexCoord[0] = vec4(t, 0.0, 0.0);
    EmitVertex();
}

void main(void)
{
    vec3 v0 = normalize(gl_PositionIn[0].xyz);
    vec3 v1 = normalize(gl_PositionIn[1].xyz);
    vec3 v2 = normalize(gl_PositionIn[2].xyz);

    vec3 v3 = normalize(v1 + v2);
    vec3 v4 = normalize(v2 + v0);
    vec3 v5 = normalize(v0 + v1);

    vec2 t0 = gl_TexCoordIn[0][0].st;
    vec2 t1 = gl_TexCoordIn[1][0].st;
    vec2 t2 = gl_TexCoordIn[2][0].st;

    vec2 t3 = (t1 + t2) * 0.5;
    vec2 t4 = (t2 + t0) * 0.5;
    vec2 t5 = (t0 + t1) * 0.5;

    emit(v2, t2);
    emit(v4, t4);
    emit(v3, t3);
    emit(v5, t5);
    emit(v1, t1);
    EndPrimitive();

    emit(v4, t4);
    emit(v0, t0);
    emit(v5, t5);
    EndPrimitive();
}