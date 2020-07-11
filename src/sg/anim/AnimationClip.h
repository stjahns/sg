#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <vector>
#include "Skeleton.h"
#include "Pose.h"

using namespace glm;

template<typename T>
struct Key
{
    Key(float time, const T& value) : time(time), value(value)
    {
    }

    float time;
    T value;
};

template<typename T>
class Channel
{
public:

    Channel() : target(BoneIndex::Invalid)
    {
    }

    Channel(BoneIndex target) : target(target)
    {
    }

    T Evaulate(float time) const;

    void AddKey(float time, T value)
    {
        keys.emplace_back(time, value);
        startTime = keys.front().time;
        float endTime = keys.back().time;
        duration = endTime - startTime;
    }

    BoneIndex GetTarget() const { return target; }

private:

    BoneIndex target; // TODO -- this is bound to a specific skeleton, can we make this safer / less coupled?
    std::vector<Key<T>> keys;

    float startTime;
    float duration;
};

typedef Channel<vec3> TranslationChannel;
typedef Channel<quat> RotationChannel;

class AnimationClip
{
public:
    void EvaulatePose(const Skeleton& skeleton, float time, Pose& pose) const;

    std::vector<TranslationChannel> translationChannels;
    std::vector<RotationChannel> rotationChannels;

    float duration;
};

