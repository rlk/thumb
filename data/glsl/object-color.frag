#version 120

uniform sampler2D       spec_map;
uniform sampler2D       diff_map;
uniform sampler2D       norm_map;

uniform sampler2DShadow shadow[3];

varying vec4 fV;
varying vec4 fL[4];
varying vec3 fD[4];
varying vec4 fS[4];

const vec3 Kd = vec3(0.5, 0.5, 0.5);
const vec3 Ks = vec3(1.0, 1.0, 1.0);
const vec3 Ka = vec3(0.2, 0.2, 0.2);

float splitc(float k, float n, float f)
{
    return mix(n * pow(f / n, k), n + (f - n) * k, 0.5);
}

float splitz(float k, float n, float f)
{
    float c = splitc(k, n, f);
    return (f / c) * (c - n) / (f - n);
}

void main()
{
    float n = gl_ClipPlane[0].w;
    float f = gl_ClipPlane[1].w;

    vec4 Ts = texture2D(spec_map, gl_TexCoord[0].xy);
    vec4 Td = texture2D(diff_map, gl_TexCoord[0].xy);
    vec4 Tn = texture2D(norm_map, gl_TexCoord[0].xy);

    vec3  V = normalize(-fV.xyz);
    vec3  N = normalize(2.0 * Tn.rgb - 1.0);

    vec3 Ca = Ka * Td.rgb;
    vec3 Cd = vec3(0.0);
    vec3 Cs = vec3(0.0);

    for (int i = 0; i < 1; i++)
    {
        // float k0 = gl_LightSource[i].ambient.x;
        // float k1 = gl_LightSource[i].ambient.y;

        float r =    length(fL[i].xyz - fV.xyz);
        vec3  L = normalize(fL[i].xyz - fV.xyz);
        vec3  D = normalize(fD[i]);
        vec3  R = reflect(L, N);

        float a = 1.0 / (gl_LightSource[i].constantAttenuation  +
                         gl_LightSource[i].linearAttenuation    * r +
                         gl_LightSource[i].quadraticAttenuation * r * r);

        vec4  Cl = gl_LightSource[i].diffuse;
        float Nl = gl_LightSource[i].spotExponent;
        float Dl = gl_LightSource[i].spotCosCutoff;

        float c = step(0.0, L.z)
                *      pow(dot(L, -D), Nl)
                * step(Dl, dot(L, -D));

        float S = a * c * shadow2DProj(shadow[i], fS[i]).r;

        float Ns = max(1.0, Ts.a * 64.0);

        Cs += S * Ks * pow(max(dot(V, R), 0.0), Ns) * Ts.rgb * Cl.rgb;
        Cd += S * Kd *     max(dot(L, N), 0.0)      * Td.rgb * Cl.rgb;
    }

    gl_FragColor = vec4(Ca + Cd + Cs, Td.a);
}
