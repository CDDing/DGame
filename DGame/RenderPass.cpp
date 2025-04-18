#include "pch.h"
#include "RenderPass.h"
vk::Format DDing::ForwardPass::DepthFormat = vk::Format::eD32Sfloat;
DDing::ForwardPass::ForwardPass(Pipeline& pipeline, vk::RenderPass renderPass)
	:RenderPass(pipeline,renderPass)
{
}

void DDing::ForwardPass::Render(vk::CommandBuffer commandBuffer, DDing::Scene& scene)
{

}

void DDing::ForwardPass::createDepthImage()
{
	
	vk::ImageCreateInfo createInfo{};
	createInfo.setFormat(ForwardPass::DepthFormat);

	VmaAllocationCreateInfo allocInfo{};
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	allocInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	VkImage imageVal = depthImage.image;
	//vmaCreateImage(d, createInfo, allocInfo, &imageVal, depthImage.allocation, nullptr);
	
	vk::ImageViewCreateInfo viewInfo{};
	viewInfo.setImage(depthImage.image);

}

void DDing::ForwardPass::createOutputImages()
{
}

void DDing::ForwardPass::createFramebuffers()
{
}

DDing::DeferredPass::DeferredPass(Pipeline& pipeline, vk::RenderPass renderPass)
	:RenderPass(pipeline, renderPass)
{

}

void DDing::DeferredPass::Render(vk::CommandBuffer commandBuffer, DDing::Scene& scene)
{

}
