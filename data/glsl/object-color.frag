#version 120

uniform sampler2D       spec_map;
uniform sampler2D       diff_map;
uniform sampler2D       norm_map;

uniform sampler2DShadow shadow[4];

varying vec4 fV;
varying vec4 fZ;
varying vec4 fL[4];
varying vec3 fD[4];
varying vec4 fS[4];

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

vec3 slight(vec3 V, vec3 N, vec4 Td, vec4 Ts, int i)
{
    float r =    length(fL[i].xyz - fV.xyz);
    vec3  L = normalize(fL[i].xyz - fV.xyz);
    vec3  D = normalize(fD[i]);
    vec3  R = reflect(L, N);

    float ld = dot(L, -D);

    vec3  Cl = gl_LightSource[i].diffuse.rgb
            / (gl_LightSource[i].constantAttenuation  +
               gl_LightSource[i].linearAttenuation    * r +
               gl_LightSource[i].quadraticAttenuation * r * r);

    float spot = step(gl_LightSource[i].spotCosCutoff, ld) *
              pow(ld, gl_LightSource[i].spotExponent);

    float ks = pow(max(dot(V, R), 0.0), Ts.a * 64.0);
    float kd =     max(dot(L, N), 0.0);

    float S = shadow2DProj(shadow[i], fS[i]).r;

    return Cl * spot * S * (Td.rgb * kd + Ts.rgb * ks);
}

vec3 dlight(vec3 V, vec3 N, vec4 Td, vec4 Ts, int i)
{
    float z0 = splitz(gl_LightSource[i].ambient.x, gl_ClipPlane[0].w, gl_ClipPlane[1].w);
    float z1 = splitz(gl_LightSource[i].ambient.y, gl_ClipPlane[0].w, gl_ClipPlane[1].w);

    vec3  L = normalize(fL[i].xyz - fZ.xyz);
    vec3  R = reflect(L, N);

    vec3  Cl = gl_LightSource[i].diffuse.rgb;

    float split = step(z0, gl_FragCoord.z)
                * step(gl_FragCoord.z, z1);

    float ks = pow(max(dot(V, R), 0.0), Ts.a * 64.0);
    float kd =     max(dot(L, N), 0.0);

    float S = shadow2DProj(shadow[i], fS[i]).r;

    return Cl * split * S * (Td.rgb * kd + Ts.rgb * ks);
}

void main()
{
    vec4 Td = texture2D(diff_map, gl_TexCoord[0].xy);
    vec4 Ts = texture2D(spec_map, gl_TexCoord[0].xy);
    vec4 Tn = texture2D(norm_map, gl_TexCoord[0].xy);

    vec3 V = normalize(-fV.xyz);
    vec3 N = normalize(2.0 * Tn.rgb - 1.0);

    vec3 C = Ka * Td.rgb
           + mix(dlight(V, N, Td, Ts, 0), slight(V, N, Td, Ts, 0), gl_LightSource[0].position.w)
           + mix(dlight(V, N, Td, Ts, 1), slight(V, N, Td, Ts, 1), gl_LightSource[1].position.w)
           + mix(dlight(V, N, Td, Ts, 2), slight(V, N, Td, Ts, 2), gl_LightSource[2].position.w)
           + mix(dlight(V, N, Td, Ts, 3), slight(V, N, Td, Ts, 3), gl_LightSource[3].position.w)
           ;
    gl_FragColor = vec4(C, Td.a);
}