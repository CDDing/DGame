#include "pch.h"
#include "RenderPass.h"
vk::Format DDing::ForwardPass::DepthFormat = vk::Format::eD32Sfloat;
vk::Format DDing::ForwardPass::ColorFormat = vk::Format::eB8G8R8A8Unorm;
DDing::ForwardPass::ForwardPass(Pipeline& pipeline, vk::RenderPass renderPass)
	:RenderPass(pipeline,renderPass)
{
	createDepthImage();
	createOutputImages();
	createFramebuffers();
}

void DDing::ForwardPass::Render(vk::CommandBuffer commandBuffer, DDing::Scene& scene)
{
	std::array<vk::ClearValue, 2> clearValues{};
	clearValues[0].color = vk::ClearColorValue{1.0f, 1.0f, 1.0f, 1.0f};
	clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);

	vk::RenderPassBeginInfo renderPassbeginInfo{};
	renderPassbeginInfo.setRenderPass(renderPass);
	renderPassbeginInfo.setClearValues(clearValues);
	renderPassbeginInfo.setFramebuffer(framebuffers[DGame->render.currentFrame]);
	renderPassbeginInfo.setRenderArea(vk::Rect2D(vk::Offset2D(0, 0), DGame->swapChain.extent));
	
	vk::Viewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(DGame->swapChain.extent.width);
	viewport.height = static_cast<float>(DGame->swapChain.extent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	vk::Rect2D scissor{};
	scissor.offset = vk::Offset2D(0, 0);
	scissor.extent = DGame->swapChain.extent;

	commandBuffer.setViewport(0, viewport);
	commandBuffer.setScissor(0, scissor);

	commandBuffer.beginRenderPass(renderPassbeginInfo, vk::SubpassContents::eInline);
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline);
	
	commandBuffer.draw(3, 1, 0, 0);

	commandBuffer.endRenderPass();

}

void DDing::ForwardPass::createDepthImage()
{
	vk::ImageCreateInfo imageInfo{};
	imageInfo.setArrayLayers(1);
	imageInfo.setExtent({ DGame->swapChain.extent.width, DGame->swapChain.extent.height, 1 });
	imageInfo.setFormat(DDing::ForwardPass::DepthFormat);
	imageInfo.setImageType(vk::ImageType::e2D);
	imageInfo.setMipLevels(1);
	imageInfo.setSamples(vk::SampleCountFlagBits::e1);
	imageInfo.setTiling(vk::ImageTiling::eOptimal);
	imageInfo.setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment);
	imageInfo.setInitialLayout(vk::ImageLayout::eUndefined);
	imageInfo.setSharingMode(vk::SharingMode::eExclusive);

	vk::ImageViewCreateInfo imageViewInfo{};
	imageViewInfo.setFormat(imageInfo.format);
	imageViewInfo.setViewType(vk::ImageViewType::e2D);
	imageViewInfo.setSubresourceRange({ vk::ImageAspectFlagBits::eDepth, 0, imageInfo.mipLevels, 0, 1 });

	VmaAllocationCreateInfo allocCreateInfo = {};
	allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
	allocCreateInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
	allocCreateInfo.priority = 1.0f;

	depthImage = DDing::Image(imageInfo, allocCreateInfo, imageViewInfo);
}

void DDing::ForwardPass::createOutputImages()
{
	for (int i = 0; i < FRAME_CNT; i++) {
		vk::ImageCreateInfo imageInfo{};
		imageInfo.setArrayLayers(1);
		imageInfo.setExtent({ DGame->swapChain.extent.width, DGame->swapChain.extent.height, 1 });
		imageInfo.setFormat(DDing::ForwardPass::ColorFormat);
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

void DDing::ForwardPass::createFramebuffers()
{
	for (int i = 0; i < FRAME_CNT; i++) {
		std::array<vk::ImageView, 2> attachments = {
			outputImages[i].imageView,
			depthImage.imageView,
		};
		vk::FramebufferCreateInfo framebufferInfo{};
		framebufferInfo.setRenderPass(renderPass);
		framebufferInfo.setAttachments(attachments);
		framebufferInfo.setWidth(DGame->swapChain.extent.width);
		framebufferInfo.setHeight(DGame->swapChain.extent.height);
		framebufferInfo.setLayers(1);
		
		framebuffers.emplace_back(DGame->context.logical,framebufferInfo);
	}
}

DDing::DeferredPass::DeferredPass(Pipeline& pipeline, vk::RenderPass renderPass)
	:RenderPass(pipeline, renderPass)
{

}

void DDing::DeferredPass::Render(vk::CommandBuffer commandBuffer, DDing::Scene& scene)
{

}

DDing::Image& DDing::RenderPass::GetOutputImage()
{
	return outputImages[DGame->render.currentFrame];
}
