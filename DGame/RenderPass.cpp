#include "pch.h"
#include "RenderPass.h"
#include "Transform.h"
#include "Light.h"



void DDing::RenderPass::createOutputImages()
{
	for (int i = 0; i < FRAME_CNT; i++) {
		vk::ImageCreateInfo imageInfo{};
		imageInfo.setArrayLayers(1);
		imageInfo.setExtent({ DGame->swapChain.extent.width, DGame->swapChain.extent.height, 1 });
		imageInfo.setFormat(vk::Format::eB8G8R8A8Unorm);
		imageInfo.setImageType(vk::ImageType::e2D);
		imageInfo.setMipLevels(1);
		imageInfo.setSamples(vk::SampleCountFlagBits::e1);
		imageInfo.setTiling(vk::ImageTiling::eOptimal);
		imageInfo.setUsage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc);
		imageInfo.setInitialLayout(vk::ImageLayout::eUndefined);
		imageInfo.setSharingMode(vk::SharingMode::eExclusive);

		vk::ImageViewCreateInfo imageViewInfo{};
		imageViewInfo.setFormat(imageInfo.format);
		imageViewInfo.setViewType(vk::ImageViewType::e2D);
		imageViewInfo.setSubresourceRange({ vk::ImageAspectFlagBits::eColor, 0, imageInfo.mipLevels, 0, 1 });

		VmaAllocationCreateInfo allocCreateInfo = {};
		allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
		allocCreateInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
		allocCreateInfo.priority = 1.0f;

		outputImages.emplace_back(imageInfo, allocCreateInfo, imageViewInfo);
	}
}



DDing::Image& DDing::RenderPass::GetOutputImage()
{
	return outputImages[DGame->render.currentFrame];
}
