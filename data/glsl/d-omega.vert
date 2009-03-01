
varying vec3 V_v;

void main()
{
    V_v = gl_Vertex.xyz;

    // Transform texture coordinates [0,1] to clip space coordinates [-1,+1].

    gl_Position = vec4(gl_MultiTexCoord0.xy * 2.0 - 1.0, 1.0, 1.0);
}
