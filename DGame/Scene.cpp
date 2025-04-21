#include "pch.h"
#include "Scene.h"
DDing::Scene::Scene() {
	camera = std::make_unique<DDing::GameObject>();
	camera->AddComponent<Camera>();
}

void DDing::Scene::AddRootNode(DDing::GameObject* rootNode)
{
	rootNodes.push_back(rootNode);
}

void DDing::Scene::AddNode(std::unique_ptr<DDing::GameObject>& node)
{
	nodes.push_back(std::move(node));
}


void DDing::Scene::Update()
{
	camera->Update();
	for (auto& node : rootNodes) {
		node->Update();
	}
}
