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

    Channel() 
        : target(BoneIndex::Invalid)
        , startTime(0.0f)
        , duration(0.0f)
    {
    }

    Channel(BoneIndex target) 
        : target(target)
        , startTime(0.0f)
        , duration(0.0f)
    {
    }

    T Evaulate(float time) const;

    T EvaluateVariableSampleRate(float time) const
    {
        if (keys.size() == 1)
        {
            return keys[0].value;
        }
        else if (keys.size() > 1)
        {
            for (size_t i = 0; i < keys.size() - 1; ++i)
            {
                const Key<T>& key1 = keys[i];
                const Key<T>& key2 = keys[i + 1];

                if (key1.time <= time && key2.time >= time)
                {
                    float t = (time - key1.time) / (key2.time - key1.time);
                    return mix(key1.value, key2.value, t);
                }
            }
        }

        return T();
    }

    void Resample()
    {
        if (keys.size() <= 2)
        {
            return;
        }

        float minInterval = FLT_MAX;
        for (int i = 0; i < keys.size() - 2; ++i)
        {
            float interval = keys[i + 1].time - keys[i].time;

            if (interval == 0.0f)
            {
                continue;
            }

            minInterval = min(minInterval, interval);
        }

        // just to make sure?
        if (minInterval == FLT_MAX || minInterval == 0.0f)
        {
            return;
        }

        std::vector<Key<T>> resampledKeys;
        for (float time = 0.0f; time <= duration; time += minInterval)
        {
            T resampledValue = EvaluateVariableSampleRate(time);
            resampledKeys.emplace_back(time, resampledValue);
        }

        keys = std::move(resampledKeys);
    }

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

