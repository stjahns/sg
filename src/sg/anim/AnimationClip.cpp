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
        pose.localTransforms[channel.GetTarget()] = mat3_cast(value);
    }

    for (const auto& channel : translationChannels)
    {
        vec3 value = channel.Evaulate(time);
        pose.localTransforms[channel.GetTarget()][3][0] = value.x;
        pose.localTransforms[channel.GetTarget()][3][1] = value.y;
        pose.localTransforms[channel.GetTarget()][3][2] = value.z;
    }
}

template <typename T>
T Channel<T>::Evaulate(float time) const
{
    if (keys.size() == 0)
    {
        return T();
    }
    if (keys.size() == 1)
    {
        return keys[0].value;
    }
    else
    {
        int index = int((time - startTime) * keys.size() / duration);
        index = clamp<int>(index, 0, keys.size() - 2);

        const Key<T>& key1 = keys[index];
        const Key<T>& key2 = keys[index + 1];

        float t = (time - key1.time) / (key2.time - key1.time);
        return mix(key1.value, key2.value, t);
    }
}
