#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

struct EntityTransform
{
    vec3 position;
    quat orientation;

    mat4 ToMat4()
    {
        mat4 matrix = mat4_cast(orientation);
        matrix[3] = vec4(position, 1.0f);
        return matrix;
    }
};

struct EntityModel
{
    int modelIndex;
};

struct AnimGraph
{
    AnimGraph(std::unique_ptr<AnimationNode> node) : rootNode(std::move(node))
    {
    }

    std::unique_ptr<AnimationNode> rootNode;
};

typedef vec3 Velocity;
