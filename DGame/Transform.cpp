#include "pch.h"
#include "Transform.h"

DDing::Transform::Transform(){
    RecalculateMatrices();
}

DDing::Transform::~Transform()
{
}


void DDing::Transform::Update()
{

}

bool DDing::Transform::DecomposeMatrix(const glm::mat4& matrix, glm::vec3& position, glm::quat& rotation, glm::vec3& scale)
{
    glm::vec3 skew;
    glm::vec4 perspective;
    return glm::decompose(matrix, scale, rotation, position, skew, perspective);
}

void DDing::Transform::MakeDirty()
{
    if (isDirty)
        return;

    isDirty = true;
    for (auto child : children)
        child->MakeDirty();
}

void DDing::Transform::RecalculateMatrices()
{
    if (!isDirty) return;

    cachedLocalMatrix =  glm::scale(glm::translate(glm::mat4(1.0f), localPosition) *
            glm::mat4(localRotation), localScale);

    if (parent) {
        cachedWorldMatrix = parent->GetWorldMatrix() * cachedLocalMatrix;
    }
    else {
        cachedWorldMatrix = cachedLocalMatrix;
    }

    DecomposeMatrix(cachedWorldMatrix, worldPosition, worldRotation, worldScale);

    isDirty = false;

}
