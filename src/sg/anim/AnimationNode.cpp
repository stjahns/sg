#include "AnimationNode.h"
#include "BoneIndex.h"

void ClipNode::Evaluate(AnimationPose& pose)
{
    for (const auto& channel : clip.rotationChannels)
    {
        quat value = channel.Evaulate(time);
        pose.SetRotation(channel.target, value);
    }

    // TODO -- assimp is adding translation channels that aren't actually in the GLTF
    // Can/should we avoid blending translations that are just the default bind pose (profile!)
    for (const auto& channel : clip.translationChannels)
    {
        vec3 value = channel.Evaulate(time);
        pose.SetTranslation(channel.target, value);
    }
}

namespace
{
    void BlendTranslation(AnimationPose& output, AnimationPose& input1, AnimationPose& input2, BoneIndex i, float blend)
    {
        vec3 translation1, translation2;
        bool input1valid = input1.GetTranslation(i, translation1);
        bool input2valid = input2.GetTranslation(i, translation2);

        if (input1valid && input2valid)
        {
            output.SetTranslation(i, mix(translation1, translation2, blend));
        }
        else if (input1valid)
        {
            output.SetTranslation(i, translation1);
        }
        else if (input2valid)
        {
            output.SetTranslation(i, translation2);
        }
    }

    void BlendRotation(AnimationPose& output, AnimationPose& input1, AnimationPose& input2, BoneIndex i, float blend)
    {
        quat rotation1, rotation2;
        bool input1valid = input1.GetRotation(i, rotation1);
        bool input2valid = input2.GetRotation(i, rotation2);

        if (input1valid && input2valid)
        {
            output.SetRotation(i, mix(rotation1, rotation2, blend));
        }
        else if (input1valid)
        {
            output.SetRotation(i, rotation1);
        }
        else if (input2valid)
        {
            output.SetRotation(i, rotation2);
        }
    }
}

void BlendNode::Evaluate(AnimationPose& output)
{
    AnimationPose input1, input2;

    node1.Evaluate(input1);
    node2.Evaluate(input2);

    int numBones = max(input1.GetBoneCount(), input2.GetBoneCount());
    for (int i = 0; i < numBones; ++i)
    {
        BlendRotation(output, input1, input2, i, blend);
        BlendTranslation(output, input1, input2, i, blend);
    }
}
