
uniform vec3 corner_a;
uniform vec3 corner_b;
uniform vec3 corner_c;
uniform vec3 corner_d;

void main()
{
    vec3 t = mix(corner_a, corner_b, gl_Vertex.x);
    vec3 u = mix(corner_c, corner_d, gl_Vertex.x);

    vec3 v = normalize(mix(t, u, gl_Vertex.y));
    
    gl_TexCoord[0] = gl_Vertex;
    gl_Position    = gl_ModelViewProjectionMatrix * vec4(v, 1.0);
}
