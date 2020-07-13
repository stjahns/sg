#pragma once

#include "assimp/matrix4x4.h"
#include "Skeleton.h"

#include <glm/glm.hpp>
#include <assimp/scene.h>

#include <string>
#include <vector>

glm::mat4 ConvertToMat4(aiMatrix4x4 in);

bool LoadSkeleton(const aiScene& scene, Skeleton& skeleton);
