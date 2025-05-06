#include "pch.h"
#include "Scene.h"
#include "ResourceManager.h"
DDing::Scene::Scene() {
	camera = std::make_unique<DDing::GameObject>();
	camera->AddComponent<Camera>();
}

void DDing::Scene::LoadSceneFromGLTF(const LoadedGLTF& gltf)
{
	for (const auto& rootNode : gltf.rootNodes) {
		rootNodes.push_back(rootNode);
	}
}

void DDing::Scene::AddRootNode(DDing::GameObject* rootNode)
{
	rootNodes.push_back(rootNode);
}

void DDing::Scene::AddRootNode(std::unique_ptr<DDing::GameObject> rootNode)
{
	rootNodes.push_back(rootNode.get());
	nodes.push_back(std::move(rootNode));
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
