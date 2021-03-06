#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <assimp/scene.h>

#include <string>
#include <vector>

#include "Skeleton.h"
#include "AnimationClip.h"

using namespace glm;

bool LoadClip(const aiScene& scene, Skeleton& skeleton, AnimationClip& clip, int clipIndex, std::string idPrefix)
{
	const aiAnimation& animation = *scene.mAnimations[clipIndex];

	double timeScale = 1.0;

	if (animation.mTicksPerSecond != 0)
	{
        // we will normalize time to seconds in the loaded clips
        timeScale /= animation.mTicksPerSecond;
	}

	clip.duration = animation.mDuration * timeScale;

	for (int channelIndex = 0; channelIndex < animation.mNumChannels; ++channelIndex)
	{
		const aiNodeAnim& nodeAnim = *animation.mChannels[channelIndex];
        BoneIndex boneIndex = skeleton.FindBoneIndex(idPrefix + nodeAnim.mNodeName.C_Str());
		if (boneIndex.IsValid())
		{
			if (nodeAnim.mNumPositionKeys > 0)
			{
				TranslationChannel channel(boneIndex);

				for (int keyIndex = 0; keyIndex < nodeAnim.mNumPositionKeys; ++keyIndex)
				{
					const aiVectorKey& key = nodeAnim.mPositionKeys[keyIndex];
					const vec3 value(key.mValue.x, key.mValue.y, key.mValue.z);
					channel.AddKey(key.mTime * timeScale, value);
				}

				channel.Resample();

				clip.translationChannels.push_back(channel);
			}

			if (nodeAnim.mNumRotationKeys > 0)
			{
				RotationChannel channel(boneIndex);

				for (int keyIndex = 0; keyIndex < nodeAnim.mNumRotationKeys; ++keyIndex)
				{
					const aiQuatKey& key = nodeAnim.mRotationKeys[keyIndex];
					const quat value(key.mValue.w, key.mValue.x, key.mValue.y, key.mValue.z);
					channel.AddKey(key.mTime * timeScale, value);
				}

				channel.Resample();

				clip.rotationChannels.push_back(channel);
			}
		}
	}

	return true;
}
