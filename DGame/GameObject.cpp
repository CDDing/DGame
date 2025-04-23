#include "pch.h"
#include "GameObject.h"
#include "Transform.h"
#include "MeshRenderer.h"
DDing::GameObject::GameObject()
{
    AddComponent<Transform>();
}

DDing::GameObject::~GameObject()
{
}

void DDing::GameObject::Update()
{
    for (auto& [type, component] : components) {
        component->Update();
    }

}

void DDing::GameObject::Draw(vk::CommandBuffer commandBuffer)
{
    auto meshRenderer = GetComponent<DDing::MeshRenderer>();
    //Draw
    if (meshRenderer){
        auto mesh = meshRenderer->GetMesh();
        if (!mesh)
            return;

        auto transformMatrix = GetComponent<DDing::Transform>()->GetWorldMatrix();

        mesh->Draw(commandBuffer, transformMatrix, *DGame->render.currentPipeline->GetLayout());
    }

    auto transform = GetComponent<DDing::Transform>();
    for (auto child : transform->GetChildren()) {


        assert(child->GetGameObject());
        child->GetGameObject()->Draw(commandBuffer);
    }

}
