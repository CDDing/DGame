#include "pch.h"
#include "ResourceManager.h"
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "tiny_gltf.h"
#include "GameObject.h"
#include "Transform.h"
#include "MeshRenderer.h"
std::vector<std::unique_ptr<DDing::Scene>> ResourceManager::LoadGLTF(const std::string name, const std::string path)
{
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, path);
    if (!warn.empty())
        std::cout << "Tiny Warning ! : " << warn << std::endl;

    if (!err.empty())
        std::cout << "Tiny Error ! :" << err << std::endl;

    if (!ret)
        throw std::runtime_error("Failed to Load GLTF!");
    
    for (const auto& tex : model.textures) {
        const auto& image = model.images[tex.source];
        std::string name = image.uri.empty() ? "generated_" + std::to_string(tex.source) : image.uri;


    }
    MeshContainer container;
    for (const auto& mesh : model.meshes) {
        LoadMesh(container, mesh, model);
    }
    meshes.push_back(std::move(container));

    std::vector<std::unique_ptr<DDing::Scene>> result;
    for (auto& scene : model.scenes) {
        //scene.nodes[0]
        auto newScene = std::make_unique<DDing::Scene>();

        for (auto nodeIndex : scene.nodes) {
            auto rootNode = CreateNodeRecursive(newScene, model, nodeIndex, nullptr);
            
            //Note Order
            newScene->AddRootNode(rootNode.get());
            newScene->AddNode(rootNode);
            
            
        }



        result.push_back(std::move(newScene));
    }

    return result;
}


void ResourceManager::LoadMesh(MeshContainer& meshContainer, const tinygltf::Mesh& mesh, const tinygltf::Model& model)
{



    std::vector<DDing::Vertex> vertices;
    std::vector<uint32_t> indices;
    for (const auto& primitive : mesh.primitives) {
        //Index
        {
            if (primitive.indices >= 0) {
                const auto& accessor = model.accessors[primitive.indices];
                const auto& bufferView = model.bufferViews[accessor.bufferView];
                const auto& buffer = model.buffers[bufferView.buffer];

                const uint8_t* dataPtr = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;
                size_t count = accessor.count;

                if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                    const uint16_t* buf = reinterpret_cast<const uint16_t*>(dataPtr);
                    for (size_t i = 0; i < count; ++i)
                        indices.push_back(static_cast<uint32_t>(buf[i]));
                }
                else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
                    const uint32_t* buf = reinterpret_cast<const uint32_t*>(dataPtr);
                    for (size_t i = 0; i < count; ++i)
                        indices.push_back(buf[i]);
                }
            }
        }

        //Vertex
        std::vector<glm::vec3> positions, normals;
        std::vector<glm::vec2> texcoords;
        {
            for (const auto& attr : primitive.attributes) {
                const std::string& attrName = attr.first;
                const auto& accessor = model.accessors[attr.second];
                const auto& bufferView = model.bufferViews[accessor.bufferView];
                const auto& buffer = model.buffers[bufferView.buffer];

                const uint8_t* dataPtr = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;

                if (attrName == "POSITION") {
                    for (size_t i = 0; i < accessor.count; ++i) {
                        const float* elem = reinterpret_cast<const float*>(dataPtr + i * accessor.ByteStride(bufferView));
                        positions.emplace_back(elem[0], elem[1], elem[2]);
                    }
                }
                else if (attrName == "NORMAL") {
                    for (size_t i = 0; i < accessor.count; ++i) {
                        const float* elem = reinterpret_cast<const float*>(dataPtr + i * accessor.ByteStride(bufferView));
                        normals.emplace_back(elem[0], elem[1], elem[2]);
                    }
                }
                else if (attrName == "TEXCOORD_0") {
                    for (size_t i = 0; i < accessor.count; ++i) {
                        const float* elem = reinterpret_cast<const float*>(dataPtr + i * accessor.ByteStride(bufferView));
                        texcoords.emplace_back(elem[0], elem[1]);
                    }
                }
            }

            // 3. Combine attributes into Vertex struct
            for (size_t i = 0; i < positions.size(); ++i) {
                DDing::Vertex v{};
                v.position = positions[i];
                v.normal = i < normals.size() ? normals[i] : glm::vec3(0.0f);
                v.texcoord = i < texcoords.size() ? texcoords[i] : glm::vec2(0.0f);
                vertices.push_back(v);
            }

        }

    }

    auto newMesh = std::make_unique<DDing::Mesh>(vertices,indices);
    meshContainer.push_back(std::move(newMesh));
}

std::unique_ptr<DDing::GameObject> ResourceManager::CreateNodeRecursive(std::unique_ptr<DDing::Scene>& scene, const tinygltf::Model& model, int nodeIndex, DDing::GameObject* parent)
{
    const auto& node = model.nodes[nodeIndex];
    auto go = std::make_unique<DDing::GameObject>();
    go->name = node.name;

    if (!node.translation.empty() || !node.rotation.empty() || !node.scale.empty()) {
        auto transform = go->GetComponent<DDing::Transform>();
        if (node.translation.size() == 3)
            transform->SetPosition({
                static_cast<float>(node.translation[0]),
                static_cast<float>(node.translation[1]),
                static_cast<float>(node.translation[2])
                });
        
        if (node.rotation.size() == 4)
            transform->SetRotation({
                static_cast<float>(node.rotation[0]),
                static_cast<float>(node.rotation[1]),
                static_cast<float>(node.rotation[2]),
                static_cast<float>(node.rotation[3])
                });

        if (node.scale.size() == 3)
            transform->SetScale({
                static_cast<float>(node.scale[0]),
                static_cast<float>(node.scale[1]),
                static_cast<float>(node.scale[2])
                });


    }

    if (parent != nullptr) {
        auto parentTransform = parent->GetComponent<DDing::Transform>();
        parentTransform->AddChild(go->GetComponent<DDing::Transform>());
    }

    if (node.mesh >= 0) {
        auto meshRenderer = go->AddComponent<DDing::MeshRenderer>();
        //TODO fix needed
        meshRenderer->SetMesh(meshes[meshes.size() - 1][node.mesh].get());
    }

    for (int child : node.children) {
        auto childNode = CreateNodeRecursive(scene,model, child, go.get());
        
        scene->AddNode(childNode);
    
    }

    return go;
}
