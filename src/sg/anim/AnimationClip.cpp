#include "AnimationClip.h"

void AnimationClip::EvaulatePose(const Skeleton& skeleton, float time, Pose& pose) const
{
    // TODO -- don't blow away existing translation if no corresponding translation channel,
    // Assimp will always provide translation keys, but we could optimize them out if they
    // are just the bind pose

    // TODO - profile data locality effect, should translation/rotation for a single node be applied at once?

    for (const auto& channel : rotationChannels)
    {
        quat value = channel.Evaulate(time);
        pose.localTransforms[channel.target] = mat3_cast(value);
    }

    for (const auto& channel : translationChannels)
    {
        vec3 value = channel.Evaulate(time);
        pose.localTransforms[channel.target][3][0] = value.x;
        pose.localTransforms[channel.target][3][1] = value.y;
        pose.localTransforms[channel.target][3][2] = value.z;
    }
}
