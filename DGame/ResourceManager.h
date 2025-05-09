#pragma once
#include "Mesh.h"
#include "tiny_gltf.h"
namespace DDing {
	class GameObject;
}
struct LoadedGLTF {
	std::vector<std::unique_ptr<DDing::Mesh>> meshes;
	std::vector<std::unique_ptr<DDing::Image>> images;
	std::vector<std::unique_ptr<DDing::GameObject>> nodes;
	std::vector<std::unique_ptr<DDing::Texture>> textures;
	std::vector<std::unique_ptr<DDing::Material>> materials;
	std::vector<vk::raii::Sampler> samplers;

	std::vector<DDing::GameObject*> rootNodes;

	vk::raii::DescriptorPool descriptorPool = nullptr;
	vk::raii::DescriptorSet descriptorSet = nullptr;
	DDing::Buffer materialBuffer;
	LoadedGLTF(const std::string path);
	void updateDescriptorSet(vk::DescriptorSet descriptorSet);
private:

	void LoadImages(const tinygltf::Model& model);
	void LoadMeshes(const tinygltf::Model& model);
	void LoadNodes(const tinygltf::Model& model);
	void LoadTextures(const tinygltf::Model& model);
	void LoadMaterials(const tinygltf::Model& model);
	void LoadSamplers(const tinygltf::Model& model);
	void InitBuffer();

};
class ResourceManager
{
public:
	void Init();

	//Temp
	std::vector<LoadedGLTF> gltfs;
private:

};

