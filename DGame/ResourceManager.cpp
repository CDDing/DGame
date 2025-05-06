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
    std::string lower = path;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    bool ret = false;
    if (lower.size() >= 5 && lower.substr(lower.size() - 5) == ".gltf") {
        ret = loader.LoadASCIIFromFile(&model, &err, &warn, path);
    }
    else if (lower.size() >= 4 && lower.substr(lower.size() - 4) == ".glb") {
        ret = loader.LoadBinaryFromFile(&model, &err, &warn, path);
    }


    if (!warn.empty())
        std::cout << "Tiny Warning ! : " << warn << std::endl;

    if (!err.empty())
        std::cout << "Tiny Error ! :" << err << std::endl;

    if (!ret)
        throw std::runtime_error("Failed to Load GLTF!");

    LoadImages(model);
    LoadSamplers(model);
    LoadTextures(model);
    LoadMaterials(model);
    LoadMeshes(model);
    LoadNodes(model);
    InitBuffer();
    InitDescriptorSet();
    
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
        uint32_t mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(img.width, img.height)))) + 1;

        vk::ImageCreateInfo imageCreateInfo{};
        imageCreateInfo.setArrayLayers(1);
        imageCreateInfo.setMipLevels(mipLevels);
        imageCreateInfo.setExtent({ static_cast<uint32_t>(img.width),static_cast<uint32_t>(img.height),1 });
        imageCreateInfo.setImageType(vk::ImageType::e2D);
        imageCreateInfo.setTiling(vk::ImageTiling::eOptimal);
        imageCreateInfo.setUsage(vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc);
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
            image->generateMipmaps(commandBuffer);
            //image->setImageLayout(commandBuffer, vk::ImageLayout::eShaderReadOnlyOptimal);

            });

        images.push_back(std::move(image));
    }
}

void LoadedGLTF::LoadMeshes(const tinygltf::Model& model)
{
    for (const auto& mesh : model.meshes) {

        auto loadedMesh = std::make_unique<DDing::Mesh>();
        for (const auto& primitive : mesh.primitives) {
            std::vector<DDing::Vertex> vertices;
            std::vector<uint32_t> indices;

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
auto loadedPrimitive = std::make_unique<DDing::Primitive>(vertices, indices);
loadedPrimitive->materialIndex = primitive.material;
loadedMesh->addPrimitive(std::move(loadedPrimitive));
        }
        meshes.push_back(std::move(loadedMesh));
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
                transform->SetLocalPosition({
                    static_cast<float>(node.translation[0]),
                    static_cast<float>(node.translation[1]),
                    static_cast<float>(node.translation[2])
                    });

            if (node.rotation.size() == 4) {
                glm::quat q = glm::quat(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]);
                transform->SetLocalRotation(q);
            }
            if (node.scale.size() == 3)
                transform->SetLocalScale({
                    static_cast<float>(node.scale[0]),
                    static_cast<float>(node.scale[1]),
                    static_cast<float>(node.scale[2])
                    });

            if (!node.matrix.empty()){
                //TODO
            }
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

void LoadedGLTF::LoadTextures(const tinygltf::Model& model)
{
    for (auto& texture : model.textures) {
        if (texture.source < 0 || texture.source >= model.images.size())
            continue;

        const auto& image = model.images[texture.source];
        auto loadedTexture = std::make_unique<DDing::Texture>();
        loadedTexture->image = images[texture.source].get();
        loadedTexture->sampler = *samplers[texture.sampler];

        

        textures.push_back(std::move(loadedTexture));
    }
}

void LoadedGLTF::LoadMaterials(const tinygltf::Model& model)
{
    for (auto& material : model.materials) {
        
        auto loadedMaterial = std::make_unique<DDing::Material>();
        
        //BaseColor
        if (material.pbrMetallicRoughness.baseColorTexture.index >= 0) {
            loadedMaterial->baseColorIndex = material.pbrMetallicRoughness.baseColorTexture.index;
        }
        else {
            loadedMaterial->baseColorFactor = glm::vec4(
                material.pbrMetallicRoughness.baseColorFactor[0],
                material.pbrMetallicRoughness.baseColorFactor[1],
                material.pbrMetallicRoughness.baseColorFactor[2],
                material.pbrMetallicRoughness.baseColorFactor[3]
                );
        }

        //MetallicRoughness
        if (material.pbrMetallicRoughness.metallicRoughnessTexture.index >= 0) {
            loadedMaterial->metallicRoughnessIndex = material.pbrMetallicRoughness.metallicRoughnessTexture.index;   
        }
        else {
            loadedMaterial->metallicFactor = material.pbrMetallicRoughness.metallicFactor;
            loadedMaterial->roughnessFactor = material.pbrMetallicRoughness.roughnessFactor;
        }

        //Normal
        if (material.normalTexture.index >= 0) {
            loadedMaterial->normalMapIndex = material.normalTexture.index;
        }

        //Emissive
        if (material.emissiveTexture.index >= 0) {
            loadedMaterial->emissiveIndex = material.emissiveTexture.index;
        }
        else {
            loadedMaterial->emissiveFactor = glm::vec3(
                material.emissiveFactor[0],
                material.emissiveFactor[1],
                material.emissiveFactor[2]
                );
        }
        
        materials.push_back(std::move(loadedMaterial));
    }
}

void LoadedGLTF::LoadSamplers(const tinygltf::Model& model)
{
    for (auto& sampler : model.samplers) {
        vk::SamplerCreateInfo samplerInfo{};
        switch (sampler.minFilter) {
        case TINYGLTF_TEXTURE_FILTER_NEAREST:
            samplerInfo.setMinFilter(vk::Filter::eNearest);
            break;
        case TINYGLTF_TEXTURE_FILTER_LINEAR:
            samplerInfo.setMinFilter(vk::Filter::eNearest);
            break;
        default:
            samplerInfo.setMinFilter(vk::Filter::eLinear);
            break;
        }
        switch (sampler.magFilter) {
        case TINYGLTF_TEXTURE_FILTER_NEAREST:
            samplerInfo.setMagFilter(vk::Filter::eNearest);
            break;
        case TINYGLTF_TEXTURE_FILTER_LINEAR:
            samplerInfo.setMagFilter(vk::Filter::eLinear);
            break;
        default:
            samplerInfo.setMagFilter(vk::Filter::eLinear);  // Default to linear if unsupported
            break;
        }

        // Set the address modes (wrap modes)
        switch (sampler.wrapS) {
        case TINYGLTF_TEXTURE_WRAP_REPEAT:
            samplerInfo.setAddressModeU(vk::SamplerAddressMode::eRepeat);
            break;
        case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT:
            samplerInfo.setAddressModeU(vk::SamplerAddressMode::eMirroredRepeat);
            break;
        case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE:
            samplerInfo.setAddressModeU(vk::SamplerAddressMode::eClampToEdge);
            break;
        default:
            samplerInfo.setAddressModeU(vk::SamplerAddressMode::eRepeat);  // Default to repeat
            break;
        }

        switch (sampler.wrapT) {
        case TINYGLTF_TEXTURE_WRAP_REPEAT:
            samplerInfo.setAddressModeV(vk::SamplerAddressMode::eRepeat);
            break;
        case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT:
            samplerInfo.setAddressModeV(vk::SamplerAddressMode::eMirroredRepeat);
            break;
        case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE:
            samplerInfo.setAddressModeV(vk::SamplerAddressMode::eClampToEdge);
            break;
        default:
            samplerInfo.setAddressModeV(vk::SamplerAddressMode::eRepeat);  // Default to repeat
            break;
        }



        //TODO
        samplerInfo.setMipmapMode(vk::SamplerMipmapMode::eLinear);
        samplerInfo.setAnisotropyEnable(vk::False);
        samplerInfo.setMaxLod(32.0f);

        auto loadedSampler = DGame->context.logical.createSampler(samplerInfo);
        samplers.push_back(std::move(loadedSampler));
    }
}

void LoadedGLTF::InitDescriptorSet()
{
    {
    std::vector<vk::DescriptorPoolSize> poolSizes = {
        vk::DescriptorPoolSize(vk::DescriptorType::eStorageBuffer,1),
        vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler,1024),
    };

    vk::DescriptorPoolCreateInfo poolInfo{};
    poolInfo.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet | vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind);
    poolInfo.setPoolSizes(poolSizes);
    poolInfo.setMaxSets(2);

    descriptorPool = DGame->context.logical.createDescriptorPool(poolInfo);
    }
    {

    std::vector<uint32_t> variableDescriptorCounts = { 1024 };
    vk::DescriptorSetVariableDescriptorCountAllocateInfo variableDescriptorCountAllocInfo{};
    variableDescriptorCountAllocInfo.setDescriptorCounts(variableDescriptorCounts);

    vk::DescriptorSetAllocateInfo allocInfo{};
    allocInfo.setDescriptorPool(*descriptorPool);
    allocInfo.setDescriptorSetCount(1);
    allocInfo.setSetLayouts(*DGame->render.bindLessLayout);
    allocInfo.setPNext(&variableDescriptorCountAllocInfo);

    descriptorSet = std::move(DGame->context.logical.allocateDescriptorSets(allocInfo).front());
    }
   
    {

        std::vector<vk::WriteDescriptorSet> writeDescriptorSets(2, {});
        vk::DescriptorBufferInfo bufferInfo{};
        bufferInfo.setOffset(0);
        bufferInfo.setBuffer(materialBuffer.buffer);
        bufferInfo.setRange(sizeof(DDing::Material) * materials.size());
        
        writeDescriptorSets[0].dstSet = *descriptorSet;
        writeDescriptorSets[0].dstBinding = 0;
        writeDescriptorSets[0].dstArrayElement = 0;
        writeDescriptorSets[0].descriptorType = vk::DescriptorType::eStorageBuffer;
        writeDescriptorSets[0].descriptorCount = 1;
        writeDescriptorSets[0].pBufferInfo = &bufferInfo;

        std::vector<vk::DescriptorImageInfo> textureDescriptors(textures.size());
        for (size_t i = 0; i < textures.size(); i++) {
            textureDescriptors[i].imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
            textureDescriptors[i].sampler = textures[i]->sampler;
            textureDescriptors[i].imageView = textures[i]->image->imageView;
        }

        writeDescriptorSets[1].dstBinding = 1;
        writeDescriptorSets[1].dstArrayElement = 0;
        writeDescriptorSets[1].descriptorType = vk::DescriptorType::eCombinedImageSampler;
        writeDescriptorSets[1].descriptorCount = static_cast<uint32_t>(textures.size());
        writeDescriptorSets[1].dstSet = *descriptorSet;
        writeDescriptorSets[1].pImageInfo = textureDescriptors.data();

        DGame->context.logical.updateDescriptorSets(writeDescriptorSets, nullptr);
    }
}

void LoadedGLTF::InitBuffer()
{
    DDing::Buffer staging;
    auto bufferSize = materials.size() * sizeof(DDing::Material);
    {
        vk::BufferCreateInfo bufferInfo{};
        bufferInfo.setUsage(vk::BufferUsageFlagBits::eTransferSrc);
        bufferInfo.setSize(bufferSize);

        VmaAllocationCreateInfo allocCreateInfo{};
        allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
        allocCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

        staging = DDing::Buffer(bufferInfo, allocCreateInfo);

        void* ptr = staging.GetMappedPtr();

        std::vector<DDing::Material> tempMaterials;
        for (auto& material : materials) {
            tempMaterials.push_back(*material);
        }

        memcpy(ptr, tempMaterials.data(), tempMaterials.size() * sizeof(DDing::Material));
    }
    {
        vk::BufferCreateInfo bufferInfo{};
        bufferInfo.setUsage(vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eStorageBuffer);
        bufferInfo.setSize(bufferSize);

        VmaAllocationCreateInfo allocCreateInfo{};
        allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
        allocCreateInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
        allocCreateInfo.priority = 1.0f;

        materialBuffer = DDing::Buffer(bufferInfo, allocCreateInfo);

        DGame->context.immediate_submit([&](vk::CommandBuffer commandBuffer) {
            vk::BufferCopy copyRegion{};
            copyRegion.setSrcOffset(0);
            copyRegion.setDstOffset(0);
            copyRegion.setSize(bufferSize);
            
            commandBuffer.copyBuffer(staging.buffer, materialBuffer.buffer, copyRegion);
            });
    }
}

void ResourceManager::Init()
{
    gltfs.push_back(LoadedGLTF("Resources/ABeautifulGame/ABeautifulGame.gltf"));
}
