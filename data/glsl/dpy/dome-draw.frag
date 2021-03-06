#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect map;

uniform mat4  proj;
varying vec3  ray;
uniform vec3  pos;
uniform vec2  siz;
uniform float rad;
uniform vec3  tk;
uniform vec3  pk;
uniform vec3  zk;

void main()
{
    const float pi = 3.14159265358979323844;

    float a = dot(ray, ray);
    float b = dot(ray, pos) * 2.0;
    float c = dot(pos, pos) - rad * rad;

    float t = max((-b + sqrt(b * b - 4.0 * a * c)) / (2.0 * a),
                  (-b - sqrt(b * b - 4.0 * a * c)) / (2.0 * a));

    vec3 v = (pos + ray * t) / rad;
    vec4 s = proj * vec4(v, 1.0);

    s = vec4(s.xyz / s.w, 1.0);

    float T = atan(-v.x, -v.z);
    float P = asin(v.y);
    float Z = v.z;

    vec3 k;

    float t0 = tk.x - tk.z * 0.5;
    float t1 = tk.x + tk.z * 0.5;
    float t2 = tk.y - tk.z * 0.5;
    float t3 = tk.y + tk.z * 0.5;

    float p0 = pk.x - pk.z * 0.5;
    float p1 = pk.x + pk.z * 0.5;
    float p2 = pk.y - pk.z * 0.5;
    float p3 = pk.y + pk.z * 0.5;

    float z0 = zk.x - zk.z * 0.5;
    float z1 = zk.x + zk.z * 0.5;
    float z2 = zk.y - zk.z * 0.5;
    float z3 = zk.y + zk.z * 0.5;

    if (tk.x > tk.y)
        k.x = smoothstep(t0, t1, T) + (1.0 - smoothstep(t2, t3, T));
    else
        k.x = smoothstep(t0, t1, T) * (1.0 - smoothstep(t2, t3, T));

    k.y = smoothstep(p0, p1, P) * (1.0 - smoothstep(p2, p3, P));
    k.z = smoothstep(z0, z1, Z) * (1.0 - smoothstep(z2, z3, Z));

    vec2 C = siz * (s.xy + 1.0) * 0.5;

    gl_FragColor = vec4(texture2DRect(map, C).rgb * k.x * k.y * k.z, 1.0);
}
