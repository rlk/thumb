
varying vec3 V_v;
varying vec3 N_v;
varying vec3 L_v;

void main()
{
    V_v = (gl_ModelViewMatrix * gl_Vertex).xyz;
    N_v =  gl_NormalMatrix * gl_Normal;
    L_v =  gl_LightSource[0].position.xyz - V_v;

    gl_Position = ftransform();
}
