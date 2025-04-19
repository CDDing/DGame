#pragma once
#include "Camera.h"
#include "GameObject.h"
namespace DDing {
	class Scene
	{
	public:
		Scene();

		void AddRootNode(std::unique_ptr<DDing::GameObject> rootNode);
		void Update();

	private:
		std::unique_ptr<DDing::GameObject> camera;

		std::vector<std::unique_ptr<DDing::GameObject>> rootNodes;
	};
}

