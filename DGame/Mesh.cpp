#include "pch.h"
#include "Mesh.h"
#include "ForwardPass.h"
DDing::Primitive::Primitive(std::vector<Vertex> vertices, std::vector<uint32_t> indices)
	:vertices(vertices), indices(indices)
{
	const size_t vertexBufferSize = vertices.size() * sizeof(Vertex);
	const size_t indexBufferSize = indices.size() * sizeof(uint32_t);

	//Buffer 
	{
		//Vertex
		{
			vk::BufferCreateInfo bufferInfo{};
			bufferInfo.setSize(vertexBufferSize);
			bufferInfo.setUsage(vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress);

			VmaAllocationCreateInfo allocInfo{};
			allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
			allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
			allocInfo.priority = 1.0f;

			vertexBuffer = Buffer(bufferInfo, allocInfo);
			//Get Address
			vk::BufferDeviceAddressInfo deviceAddressInfo{};
			deviceAddressInfo.setBuffer(vertexBuffer.buffer);
			vertexBufferAddress = DGame->context.logical.getBufferAddress(deviceAddressInfo);
		}
		//Index
		{
			vk::BufferCreateInfo bufferInfo{};
			bufferInfo.setSize(indexBufferSize);
			bufferInfo.setUsage(vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eIndexBuffer);

			VmaAllocationCreateInfo allocInfo{};
			allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
			allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
			allocInfo.priority = 1.0f;

			indexBuffer = Buffer(bufferInfo, allocInfo);
		}
		
	}
	//Staging & Copy
	{
		vk::BufferCreateInfo bufferInfo{};
		bufferInfo.setSize(vertexBufferSize + indexBufferSize);
		bufferInfo.setUsage(vk::BufferUsageFlagBits::eTransferSrc);

		VmaAllocationCreateInfo allocInfo{};
		allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
		allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

		DDing::Buffer staging = DDing::Buffer(bufferInfo, allocInfo);

		void* data = staging.GetMappedPtr();

		memcpy(data, vertices.data(), vertexBufferSize);
		memcpy((char*)data + vertexBufferSize, indices.data(), indexBufferSize);

		DGame->context.immediate_submit([&](vk::CommandBuffer commandBuffer) {
			vk::BufferCopy vertexCopy{};
			vertexCopy.dstOffset = 0;
			vertexCopy.srcOffset = 0;
			vertexCopy.size = vertexBufferSize;

			commandBuffer.copyBuffer(staging.buffer, vertexBuffer.buffer, vertexCopy);

			vk::BufferCopy indexCopy{};
			indexCopy.dstOffset = 0;
			indexCopy.srcOffset = vertexBufferSize;
			indexCopy.size = indexBufferSize;

			commandBuffer.copyBuffer(staging.buffer, indexBuffer.buffer, indexCopy);
			});
	}
}

void DDing::Primitive::Draw(vk::CommandBuffer commandBuffer, const glm::mat4& transform, const vk::PipelineLayout pipelineLayout)
{
	DDing::ForwardPass::PushConstant pushConstant{
		transform,
		vertexBufferAddress, // primitive 고유 주소
		materialIndex
	};

	commandBuffer.pushConstants<ForwardPass::PushConstant>(
		pipelineLayout,
		vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
		0,
		pushConstant
	);
	commandBuffer.bindIndexBuffer(indexBuffer.buffer, 0, vk::IndexType::eUint32);
	commandBuffer.drawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
}

void DDing::Mesh::Draw(vk::CommandBuffer commandBuffer, const glm::mat4& transform, const vk::PipelineLayout pipelineLayout)
{
	for (auto& primitive : primitives)
		primitive->Draw(commandBuffer, transform, pipelineLayout);
}
