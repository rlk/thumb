#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect map;

varying vec3  ray;
uniform vec3  pos;
uniform float rad;
uniform vec3  tk;
uniform vec3  pk;

vec4 saw(vec4 t)
{
    vec4 k = fract(t);

//  return mix(-2.0 * k + 1.0, 2.0 * k - 1.0, step(0.5, k));
    return mix(2.0 * k, 2.0  - 2.0 * k, step(0.5, k));
}

vec4 line(vec4 t, vec4 dt, vec4 l, vec4 f)
{
    return 1.0 - smoothstep(dt, dt * f, saw(t / l) * l);
}

void main()
{
    /* Compute the dome coordinates of the pixel. */

    const float pi = 3.14159265358979323844;

    float a = dot(ray, ray);
    float b = dot(ray, pos) * 2.0;
    float c = dot(pos, pos) - rad * rad;

    float t = max((-b + sqrt(b * b - 4.0 * a * c)) / (2.0 * a),
                  (-b - sqrt(b * b - 4.0 * a * c)) / (2.0 * a));

    vec3 v = (pos + ray * t) / rad;

    float T = atan2(-v.x, -v.z);
    float P = asin(v.y);

    /* Compute the fades. */

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

    /* Generate the test pattern. */

    T = (-T +       pi) * 360.0 / (2.0 * pi);
    P = (+P + 0.5 * pi) * 180.0 / (      pi);

    float dT = fwidth(T);
    float dP = fwidth(P);

    vec4 langle = vec4(1.0, 5.0, 15.0, 45.0);
    vec4 lcolor = vec4(0.5, 0.8,  1.0,  1.0);
    vec4 lwidth = vec4(1.0, 2.0,  4.0,  8.0);

    vec4 tt = line(vec4(T), vec4(dT), langle, lwidth) * lcolor;
    vec4 pp = line(vec4(P), vec4(dP), langle, lwidth) * lcolor;

    float ttt = max(max(tt.x, tt.y), max(tt.z, tt.w));
    float ppp = max(max(pp.x, pp.y), max(pp.z, pp.w));

//  float X = max(ttt, ppp) * k.x * k.y;
    float X = max(ttt, ppp);

    gl_FragColor = vec4(0.5 * X, X, 0.5 * X, 1.0);
}
