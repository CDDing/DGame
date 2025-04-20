#pragma once
#include "Camera.h"
#include "GameObject.h"
namespace DDing {
	class Scene
	{
	public:
		Scene();

		void AddRootNode(DDing::GameObject* rootNode);
		void AddNode(std::unique_ptr<DDing::GameObject>& node);
		auto& GetRootNodes() { return rootNodes; }
		void Update();

	private:
		std::unique_ptr<DDing::GameObject> camera;

		std::vector<std::unique_ptr<DDing::GameObject>> nodes;
		std::vector<DDing::GameObject*> rootNodes;
	};
}

