#include "pch.h"
#include "GameObject.h"
#include "Transform.h"
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
