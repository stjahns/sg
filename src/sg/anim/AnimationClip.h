#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <vector>
#include "Skeleton.h"

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

    // TODO - check if branching in here is murdering performance!

    T Evaulate(float time) const
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

    BoneIndex target; // TODO -- this is bound to a specific skeleton, can we make this safer / less coupled?
    std::vector<Key<T>> keys;
};

typedef Channel<vec3> TranslationChannel;
typedef Channel<quat> RotationChannel;

class AnimationClip
{
public:
    void EvaulatePose(const Skeleton& skeleton, float time, Pose& pose) const;

    std::vector<TranslationChannel> positionChannels;
    std::vector<RotationChannel> rotationChannels;

    float duration;
};

