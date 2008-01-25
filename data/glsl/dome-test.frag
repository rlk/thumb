#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect map;

varying vec3  ray;
uniform vec3  pos;
uniform float rad;

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

    p.x = (-T +       pi) * 720.0 / (2.0 * pi);
    p.y = (+P + 0.5 * pi) * 360.0 / (      pi);

    gl_FragColor = texture2DRect(map, p);
}
