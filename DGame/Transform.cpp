#include "pch.h"
#include "Transform.h"

DDing::Transform::Transform()
    : position(0.0f), rotation(glm::quat(1.0f, 0.0f, 0.0f, 0.0f)), scale(1.0f, 1.0f, 1.0f) {
    UpdateLocalTransform();
}

DDing::Transform::~Transform()
{
}

//void DDing::Transform::SetPosition(const glm::vec3& newPosition)
//{
//    position = newPosition;
//    UpdateLocalTransform();
//}
//
//void DDing::Transform::SetRotation(const glm::quat& newRotation)
//{
//    rotation = newRotation;
//    UpdateLocalTransform();
//}
//
//void DDing::Transform::SetScale(const glm::vec3& newScale)
//{
//    scale = newScale;
//    UpdateLocalTransform();
//}

void DDing::Transform::Update()
{

}

void DDing::Transform::UpdateLocalTransform()
{
    localTransform = glm::translate(glm::mat4(1.0f), position) *
        glm::mat4_cast(rotation) * 
        glm::scale(glm::mat4(1.0f), scale); 
}
