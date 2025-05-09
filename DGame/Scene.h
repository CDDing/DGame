#pragma once
#include "Camera.h"
#include "GameObject.h"
struct LoadedGLTF;
namespace DDing {
	class Scene
	{
	public:
		Scene();
		void LoadSceneFromGLTF(const LoadedGLTF& gltf);
		void AddRootNode(DDing::GameObject* rootNode);
		void AddRootNode(std::unique_ptr<DDing::GameObject> rootNode);
		void AddNode(std::unique_ptr<DDing::GameObject>& node);
		auto& GetRootNodes() { return rootNodes; }
		auto& GetNodes() { return nodes; }
		auto GetCamera() { return camera.get(); }
		void Update();

	private:
		std::unique_ptr<DDing::GameObject> camera;

		std::vector<std::unique_ptr<DDing::GameObject>> nodes;
		std::vector<DDing::GameObject*> rootNodes;
	};
}

