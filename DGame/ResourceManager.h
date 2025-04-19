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
	void LoadMesh(const std::string& name, const tinygltf::Mesh& mesh, const tinygltf::Model& model);
	std::unique_ptr<DDing::GameObject> CreateNodeRecursive(const tinygltf::Model& model, int nodeIndex, DDing::GameObject* parent);
	std::unordered_map<std::string, std::unique_ptr<DDing::Mesh>> meshes;
};

