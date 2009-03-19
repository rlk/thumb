
varying vec3 V_v;

uniform vec3  light_position;
/*
uniform float light_theta; 
uniform float light_theta_cos;
*/
uniform vec3 perez[6];
uniform vec3 zenith_luminance;

uniform mat3 XYZRGB;

//-----------------------------------------------------------------------------

vec3 Ftheta(float theta_cos)
{
    return 1.0 + perez[0] * exp(perez[1] / theta_cos);
}

vec3 Fgamma(float gamma, float gamma_cos)
{
    return 1.0 + perez[3] * exp(perez[4] * gamma)
               + perez[5] * gamma_cos * gamma_cos;
}

vec3 F(float theta_cos, float gamma, float gamma_cos)
{
    return Ftheta(theta_cos) * Fgamma(gamma, gamma_cos);
}

vec3 luminance(vec3 V, vec3 L)
{
    float world_theta_cos = max(V.y, 0.0);
    float world_gamma_cos = dot(V,   L  );

    float light_theta_cos = L.y;

    float world_gamma = acos(world_gamma_cos);
    float light_theta = acos(light_theta_cos);

    vec3 num = F(world_theta_cos, world_gamma, world_gamma_cos);
    vec3 den = F(            1.0, light_theta, light_theta_cos);

    return zenith_luminance * num / den;
}

vec3 xyz_to_rgb(vec3 Y)
{
    return XYZRGB * vec3((            Y.y) * (Y.x / Y.z),
                         (            Y.x),
                         (1.0 - Y.y - Y.z) * (Y.x / Y.z));
}

//-----------------------------------------------------------------------------

void main()
{
    vec3 V = normalize(V_v);
    vec3 L = normalize(light_position);

    float k = pow(max(dot(V, L), 0.0), 1024.0);

    vec3 C = xyz_to_rgb(luminance(V, L)) / 50.0;
//  vec3 E = (1.0 - exp(-C / 2.0));
    vec3 E = pow(C, vec3(1.0/1.8));

    gl_FragColor = vec4(mix(E, vec3(1.0), k) * step(0.0, V.y), 1.0);

//  gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
}
