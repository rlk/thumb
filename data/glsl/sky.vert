
varying vec3 V_v;
varying vec3 L_v;

uniform float time;

void main()
{
    V_v = gl_Vertex.xyz;

    // Light vector is given by the first directional light source position.

    if       (gl_LightSource[0].position.w == 0.0)
        L_v = gl_LightSource[0].position.xyz;
    else  if (gl_LightSource[1].position.w == 0.0)
        L_v = gl_LightSource[1].position.xyz;
    else  if (gl_LightSource[2].position.w == 0.0)
        L_v = gl_LightSource[2].position.xyz;
    else  if (gl_LightSource[3].position.w == 0.0)
        L_v = gl_LightSource[3].position.xyz;

    // Transform texture coordinates [0,1] to clip space coordinates [-1,+1].

    gl_Position = vec4(gl_MultiTexCoord0.xy * 2.0 - 1.0, 1.0, 1.0);
}
