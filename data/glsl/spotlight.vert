
// This shader is crafty. Its ultimate goal is to scale the spotlight object
// such that its shape matches the cutoff angle of the light source. This is
// complicated by the fact that the object geometry may be statically batched
// and rendered in bulk with other light source and non-light source objects,
// while the light source parameters may be dynamic and changing each frame.
//
// Toward this end, the spotlight OBJ gives vertices defining a spotlight with
// zero cutoff. The normals give the change in vertex position to be included
// as the cutoff angle increases.
//
// This shader determines the cutoff angle for THIS light source by comparing
// the four light source units given in a uniform with the current unit given
// in texture coordinate p.
//
// Given this angle, calculate the necessary offset, and sum the position and
// normal.

uniform vec4 LightUnit;
uniform vec4 LightCutoff;

void main()
{
	float a = dot(LightCutoff, vec4(equal(LightUnit, vec4(gl_MultiTexCoord0.p))));
	float k = tan(radians(a * 0.5)) * 0.70710678;

	vec4 v = vec4(gl_Vertex.x + gl_Normal.x * k, gl_Vertex.y + gl_Normal.y,
				  gl_Vertex.z + gl_Normal.z * k, gl_Vertex.w);

	gl_TexCoord[0] = gl_MultiTexCoord0;
    gl_Position    = gl_ModelViewProjectionMatrix * v;
}
