#include <glm/glm.hpp>
#include <assimp/scene.h>

#include <string>
#include <vector>

#include "Skeleton.h"

using namespace glm;

mat4 ConvertToMat4(aiMatrix4x4 in)
{
	mat4 out;

	out[0][0] = in.a1;
	out[1][0] = in.a2;
	out[2][0] = in.a3;
	out[3][0] = in.a4;

	out[0][1] = in.b1;
	out[1][1] = in.b2;
	out[2][1] = in.b3;
	out[3][1] = in.b4;

	out[0][2] = in.c1;
	out[1][2] = in.c2;
	out[2][2] = in.c3;
	out[3][2] = in.c4;

	out[0][3] = in.d1;
	out[1][3] = in.d2;
	out[2][3] = in.d3;
	out[3][3] = in.d4;

	return out;
}

