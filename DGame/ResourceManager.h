#pragma once
#include "Mesh.h"
#include "tiny_gltf.h"
namespace DDing {
	class Texture;
	class GameObject;
}
struct LoadedGLTF {
	std::vector<std::unique_ptr<DDing::Mesh>> meshes;
	std::vector<std::unique_ptr<DDing::Image>> images;
	std::vector<std::unique_ptr<DDing::GameObject>> nodes;
	std::vector<DDing::GameObject*> rootNodes;

	LoadedGLTF(const std::string path);
private:

	void LoadImages(const tinygltf::Model& model);
	void LoadMeshes(const tinygltf::Model& model);
	void LoadNodes(const tinygltf::Model& model);
};
class ResourceManager
{
public:
	void Init();

	//Temp
	std::vector<LoadedGLTF> gltfs;
private:

};

