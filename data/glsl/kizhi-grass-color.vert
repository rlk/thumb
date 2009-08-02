
uniform mat4 shadow_matrix[3];

uniform mat4 view_matrix;
uniform mat4 view_inverse;
uniform vec3 terrain_size;

//varying vec4 V_e;
//varying vec3 V_v;

varying vec2  location;
varying float alpha;

void main()
{
    const float distance = 50.0;

    // Compute the view, normal, and tangent eye-space vectors.
    
    vec4 V_e = gl_ModelViewMatrix * gl_Vertex;

    float d = -V_e.z;
    float s = 2000.0 / d;

    // Compute the world-space view varying.

//    V_v = (view_inverse * V_e).xyz;

    // Material and shadow map texture coordinates.

    location = (gl_Vertex.xy + terrain_size.xy * 0.5) / terrain_size.xy;

    gl_TexCoord[0] = gl_MultiTexCoord0;
    gl_TexCoord[1] = shadow_matrix[0] * V_e;
    gl_TexCoord[2] = shadow_matrix[1] * V_e;
    gl_TexCoord[3] = shadow_matrix[2] * V_e;

    alpha        = max(0.0,   1.0 -   1.0 * d / distance);
    gl_PointSize = s;

    gl_Position = ftransform();
}
