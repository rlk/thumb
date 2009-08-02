
uniform mat4 shadow_matrix[3];

uniform mat4 view_matrix;
uniform mat4 view_inverse;
uniform vec3 terrain_size;

uniform sampler2D noise;

varying vec2  location;
varying float alpha;

void main()
{
    const float max_distance = 50.0;

    vec3 d = texture2D(noise, gl_Vertex.xy / vec2(256.0)).xyz;

    vec4 v = vec4(gl_Vertex.xy, gl_Vertex.zw);

    // Compute the view, normal, and tangent eye-space vectors.
    
    vec4 V_e = gl_ModelViewMatrix * v;

    float distance = -V_e.z;
    float size     = 2000.0 / distance;

    // Material and shadow map texture coordinates.

    location = (0.5 + floor(v.xy + terrain_size.xy * 0.5)) / terrain_size.xy;

    gl_TexCoord[0] = gl_MultiTexCoord0;
    gl_TexCoord[1] = shadow_matrix[0] * V_e;
    gl_TexCoord[2] = shadow_matrix[1] * V_e;
    gl_TexCoord[3] = shadow_matrix[2] * V_e;

    alpha        = max(0.0,   1.0 -   1.0 * distance / max_distance);
    gl_PointSize = size * d.z;

    gl_Position = ftransform();
}
