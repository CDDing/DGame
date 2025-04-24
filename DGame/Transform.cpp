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

void DDing::Transform::DrawUI()
{
    if (ImGui::CollapsingHeader("Transform")) {
        bool changed = false;


        // 1. Position
        glm::vec3 position = GetLocalPosition();
        if (ImGui::DragFloat3("Position", glm::value_ptr(position), 0.1f)) {
            SetLocalPosition(position);
            changed = true;
        }

        // 2. Rotation (show as Euler, convert to quaternion)
        if (ImGui::DragFloat3("Rotation", glm::value_ptr(localEulerAngle), 0.5f)) {
            glm::quat newRotation = glm::quat(glm::radians(localEulerAngle)); // degrees -> radians -> quaternion
            SetLocalRotation(newRotation);
            changed = true;
        }

        // 3. Scale
        glm::vec3 scale = GetLocalScale();
        if (ImGui::DragFloat3("Scale", glm::value_ptr(scale), 0.1f)) {
            SetLocalScale(scale);
            changed = true;
        }

        if (changed) {
            MakeDirty(); // 강제로 리프레시
        }

    }
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
