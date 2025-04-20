#pragma once
namespace DDing {
	class Texture;
	class GameObject;
}
#include "Mesh.h"
#include "tiny_gltf.h"
class ResourceManager
{
public:
	std::vector<std::unique_ptr<DDing::Scene>> LoadGLTF(const std::string name, const std::string path);

private:
	using MeshContainer = std::vector<std::unique_ptr<DDing::Mesh>>;

	void LoadMesh(MeshContainer& meshContainer, const tinygltf::Mesh& mesh, const tinygltf::Model& model);
	std::unique_ptr<DDing::GameObject> CreateNodeRecursive(std::unique_ptr<DDing::Scene>& scene, const tinygltf::Model& model, int nodeIndex, DDing::GameObject* parent);
	
	std::vector<MeshContainer> meshes;
};

