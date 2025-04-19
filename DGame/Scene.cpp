#include "pch.h"
#include "Scene.h"
DDing::Scene::Scene() {
	camera = std::make_unique<DDing::GameObject>();
	camera->AddComponent<Camera>();
}

void DDing::Scene::AddRootNode(std::unique_ptr<DDing::GameObject> rootNode)
{
	rootNodes.push_back(std::move(rootNode));
}
