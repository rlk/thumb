
attribute vec3 Tangent;

varying vec3 V_v;
varying vec3 T_v;
varying vec3 B_v;

void main()
{
    // Pass along the fragment position and tangent-space basis vectors.

    V_v = gl_Vertex.xyz;
    T_v = Tangent;
    B_v = cross(gl_Normal, Tangent);

    // Transform texture coordinates [0,1] to clip space coordinates [-1,+1].

    gl_Position = vec4(gl_MultiTexCoord0.xy * 2.0 - 1.0, 1.0, 1.0);
}
