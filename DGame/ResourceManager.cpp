#include "pch.h"
#include "ResourceManager.h"
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "tiny_gltf.h"
#include "GameObject.h"
#include "Transform.h"
#include "MeshRenderer.h"

LoadedGLTF::LoadedGLTF(const std::string path)
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

    LoadImages(model);
    LoadMeshes(model);
    LoadNodes(model);
    
}

void LoadedGLTF::LoadImages(const tinygltf::Model& model)
{
    for (const auto& img : model.images) {
        auto& rawData = img.image;
        vk::BufferCreateInfo bufferCreateInfo{};
        bufferCreateInfo.setSize(sizeof(uint8_t) * rawData.size());
        bufferCreateInfo.setUsage(vk::BufferUsageFlagBits::eTransferSrc);

        VmaAllocationCreateInfo allocCreateInfo{};
        allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
        allocCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

        DDing::Buffer staging(bufferCreateInfo, allocCreateInfo);
        memcpy(staging.GetMappedPtr(), rawData.data(), sizeof(uint8_t) * rawData.size());


        //change format if needed
        vk::Format imgFormat = vk::Format::eR8G8B8A8Unorm;
        
        vk::ImageCreateInfo imageCreateInfo{};
        imageCreateInfo.setArrayLayers(1);
        imageCreateInfo.setMipLevels(1);
        imageCreateInfo.setExtent({ static_cast<uint32_t>(img.width),static_cast<uint32_t>(img.height),1 });
        imageCreateInfo.setImageType(vk::ImageType::e2D);
        imageCreateInfo.setTiling(vk::ImageTiling::eOptimal);
        imageCreateInfo.setUsage(vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst);
        imageCreateInfo.setSharingMode(vk::SharingMode::eExclusive);
        imageCreateInfo.setSamples(vk::SampleCountFlagBits::e1);
        imageCreateInfo.setInitialLayout(vk::ImageLayout::eUndefined);
        imageCreateInfo.setFormat(imgFormat);

        VmaAllocationCreateInfo allocInfo{};
        allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
        allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
        allocInfo.priority = 1.0f;

        vk::ImageViewCreateInfo imageViewCreateInfo{};
        imageViewCreateInfo.setFormat(imgFormat);
        imageViewCreateInfo.setViewType(vk::ImageViewType::e2D);
        imageViewCreateInfo.setSubresourceRange({ vk::ImageAspectFlagBits::eColor, 0, imageCreateInfo.mipLevels,0,1 });

        auto image = std::make_unique<DDing::Image>(imageCreateInfo, allocInfo, imageViewCreateInfo);
        
        DGame->context.immediate_submit([&](vk::CommandBuffer commandBuffer) {
            vk::BufferImageCopy region{};
            region.setBufferImageHeight(0);
            region.setBufferRowLength(0);
            region.setBufferOffset(0);
            

            vk::ImageSubresourceLayers subresource{};
            subresource.setLayerCount(1);
            subresource.setBaseArrayLayer(0);
            subresource.setAspectMask(vk::ImageAspectFlagBits::eColor);
            subresource.setMipLevel(0);

            region.setImageSubresource(subresource);
            region.setImageOffset({ 0,0,0 });
            region.setImageExtent({ { static_cast<uint32_t>(img.width),static_cast<uint32_t>(img.height),1 } });
            
            image->setImageLayout(commandBuffer, vk::ImageLayout::eTransferDstOptimal);
            commandBuffer.copyBufferToImage(staging.buffer, image->image, vk::ImageLayout::eTransferDstOptimal, region);
            image->setImageLayout(commandBuffer, vk::ImageLayout::eShaderReadOnlyOptimal);
            });

        images.push_back(std::move(image));
    }
}

void LoadedGLTF::LoadMeshes(const tinygltf::Model& model)
{
    for (const auto& mesh : model.meshes) {

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
        auto mesh = std::make_unique<DDing::Mesh>(vertices, indices);
        meshes.push_back(std::move(mesh));
    }
}

void LoadedGLTF::LoadNodes(const tinygltf::Model& model)
{
    for (const auto& node : model.nodes) {
        auto gameObject = std::make_unique<DDing::GameObject>();
        gameObject->name = node.name;

        if (!node.translation.empty() || !node.rotation.empty() || !node.scale.empty()) {
            auto transform = gameObject->GetComponent<DDing::Transform>();
            if (node.translation.size() == 3)
                transform->SetPosition({
                    static_cast<float>(node.translation[0]),
                    -static_cast<float>(node.translation[1]),
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

        if (node.mesh >= 0) {
            auto meshRenderer = gameObject->AddComponent<DDing::MeshRenderer>();
            meshRenderer->SetMesh(meshes[node.mesh].get());
        }
        nodes.push_back(std::move(gameObject));
    }

    for (int i = 0; i < model.nodes.size(); i++) {
        auto parent = nodes[i].get();
        auto parentTransform = parent->GetComponent<DDing::Transform>();
        for (const auto& child : model.nodes[i].children) {
            auto childObject = nodes[child].get();

            auto childTransform = childObject->GetComponent<DDing::Transform>();

            parentTransform->AddChild(childTransform);
        }
    }
    for (auto node : model.scenes[model.defaultScene].nodes)
        rootNodes.push_back(nodes[node].get());
}

void ResourceManager::Init()
{
    gltfs.push_back(LoadedGLTF("Resources/ABeautifulGame/ABeautifulGame.gltf"));
}
