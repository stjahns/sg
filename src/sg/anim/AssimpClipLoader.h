#include <glm/glm.hpp>
#include <assimp/scene.h>

#include <string>
#include <vector>

using namespace glm;

class aiScene;
class Skeleton;
class AnimationClip;

bool LoadClip(const aiScene& scene, Skeleton& skeleton, AnimationClip& clip, int clipIndex = 0);
