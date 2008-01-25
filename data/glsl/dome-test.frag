#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect map;

varying vec3  ray;
uniform vec3  pos;
uniform float rad;
uniform vec3  tk;
uniform vec3  pk;

void main()
{
    const float pi = 3.14159265358979323844;

    float a = dot(ray, ray);
    float b = dot(ray, pos) * 2.0;
    float c = dot(pos, pos) - rad * rad;

    float t = max((-b + sqrt(b * b - 4.0 * a * c)) / (2.0 * a),
                  (-b - sqrt(b * b - 4.0 * a * c)) / (2.0 * a));

    vec3 v = (pos + ray * t) / rad;

    float T = atan2(-v.x, -v.z);
    float P = asin(v.y);

    vec2 p;
    vec2 k;

    float t0 = tk.x - tk.z * 0.5;
    float t1 = tk.x + tk.z * 0.5;
    float t2 = tk.y - tk.z * 0.5;
    float t3 = tk.y + tk.z * 0.5;

    float p0 = pk.x - pk.z * 0.5;
    float p1 = pk.x + pk.z * 0.5;
    float p2 = pk.y - pk.z * 0.5;
    float p3 = pk.y + pk.z * 0.5;

    if (tk.x > tk.y)
        k.x = smoothstep(t0, t1, T) + (1.0 - smoothstep(t2, t3, T));
    else
        k.x = smoothstep(t0, t1, T) * (1.0 - smoothstep(t2, t3, T));

    k.y = smoothstep(p0, p1, P) * (1.0 - smoothstep(p2, p3, P));

    p.x = (-T +       pi) * 720.0 / (2.0 * pi);
    p.y = (+P + 0.5 * pi) * 360.0 / (      pi);

    gl_FragColor = vec4(texture2DRect(map, p).rgb * k.x * k.y, 1.0);
}
