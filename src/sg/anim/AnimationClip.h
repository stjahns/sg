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
inline T interpolate(T a, T b, float t)
{
    return mix(a, b, t);
}

template<>
inline quat interpolate(quat a, quat b, float t)
{
    if (dot(a, b) < 0.0f)
    {
        return mix(a, -b, t);
    }
    else
    {
        return mix(a, b, t);
    }
}

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

    T Evaluate(float time) const
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
            return interpolate(key1.value, key2.value, t);
        }
    }

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
                    return interpolate(key1.value, key2.value, t);
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

    int GetKeyCount() const { return keys.size(); }
    Key<T> GetKey(int i) const { return keys[i]; }

    void Clear() { keys.clear(); }

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

// TODO -- this needs some cleanup ..

class AnimationClip
{
public:

    std::vector<TranslationChannel> translationChannels;
    std::vector<RotationChannel> rotationChannels;

    float duration;

    template<typename T>
    Channel<T>* GetChannelForTarget(std::vector<Channel<T>>& channels, BoneIndex target)
    {
        for (int i = 0; i < channels.size(); ++i)
        {
            if (channels[i].GetTarget() == BoneIndex(target))
            {
                return &channels[i];
            }
        }

        return nullptr;
    }

    void AddTranslationChannel(TranslationChannel channel)
    {
        translationChannels.push_back(std::move(channel));
    }

    void AddRotationChannel(RotationChannel channel)
    {
        rotationChannels.push_back(std::move(channel));
    }

    RotationChannel* GetRotationChannel(BoneIndex target)
    {
        return GetChannelForTarget(rotationChannels, target);
    }

    TranslationChannel* GetTranslationChannel(BoneIndex target)
    {
        return GetChannelForTarget(translationChannels, target);
    }

    void ComputeAdditiveRootMotion(TranslationChannel& rootTranslation, RotationChannel& rootRotation)
    {
        const TranslationChannel& sourceTranslation = *GetTranslationChannel(0);
        const RotationChannel& sourceRotation = *GetRotationChannel(0);

        for (int i = 0; i < sourceTranslation.GetKeyCount() - 1; ++i)
        {
            float time1 = sourceTranslation.GetKey(i).time;
            float time2 = sourceTranslation.GetKey(i + 1).time;
            float dT = time2 - time1;

            vec3 value1 = sourceTranslation.GetKey(i).value;
            vec3 value2 = sourceTranslation.GetKey(i + 1).value;
            vec3 delta = (value2 - value1) * (1.0f / dT);

            rootTranslation.AddKey(time1, delta);
        }

        for (int i = 0; i < sourceRotation.GetKeyCount() - 1; ++i)
        {
            float time1 = sourceRotation.GetKey(i).time;
            float time2 = sourceRotation.GetKey(i + 1).time;
            float dT = time2 - time1;

            quat value1 = sourceRotation.GetKey(i).value;
            quat value2 = sourceRotation.GetKey(i + 1).value;
            quat delta = normalize((value2 * conjugate(value1)) * (1.0f / dT));

            rootRotation.AddKey(time1, delta);
        }
    }
};
