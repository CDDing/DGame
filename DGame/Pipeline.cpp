#include "pch.h"
#include "Pipeline.h"


DDing::GraphicsPipeline::GraphicsPipeline(DDing::Context& context, PipelineDesc& desc)
{
	pipelineLayout = context.logical.createPipelineLayout(desc.layout);
	vk::GraphicsPipelineCreateInfo pipelineInfo{};
	pipeline = context.logical.createGraphicsPipeline(nullptr,pipelineInfo);
}

std::vector<uint32_t> loadShader(const char* filePath)
{
	std::ifstream file(filePath, std::ios::ate | std::ios::binary);
	if (!file.is_open())
		std::runtime_error("failed to load shader!");

	size_t fileSize = (size_t)file.tellg();
	std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));
	file.seekg(0);
	file.read((char*)buffer.data(), fileSize);
	file.close();

	return buffer;
}
