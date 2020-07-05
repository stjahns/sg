#pragma once

#include "assimp/matrix4x4.h"

class aiScene;
class Skeleton;

glm::mat4 ConvertToMat4(aiMatrix4x4 in);
bool LoadSkeleton(const aiScene& scene, Skeleton& skeleton);
