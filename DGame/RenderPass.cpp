#include "pch.h"
#include "RenderPass.h"
vk::Format DDing::ForwardPass::DepthFormat = vk::Format::eD32Sfloat;
vk::Format DDing::ForwardPass::ColorFormat = vk::Format::eB8G8R8A8Unorm;
DDing::ForwardPass::ForwardPass(Pipeline& pipeline, vk::RenderPass renderPass)
	:RenderPass(pipeline,renderPass)
{
	createDepthImage();
	createOutputImages();
	initDepthImageGUI();

	createFramebuffers();
	
}

DDing::ForwardPass::~ForwardPass()
{
	for (auto& descriptorSet : depthImageDescriptorSet) {
		ImGui_ImplVulkan_RemoveTexture(descriptorSet);
	}
}

void DDing::ForwardPass::Render(vk::CommandBuffer commandBuffer, DDing::Scene* scene)
{
	auto currentFrame = DGame->render.currentFrame;

	
	std::array<vk::ClearValue, 3> clearValues{};
	clearValues[0].color = vk::ClearColorValue{ 1.0f, 1.0f, 1.0f, 1.0f };
	clearValues[1].color = vk::ClearColorValue{ 1.0f, 1.0f, 1.0f, 1.0f };
	clearValues[2].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);


	
	vk::RenderPassBeginInfo renderPassbeginInfo{};
	renderPassbeginInfo.setRenderPass(renderPass);
	renderPassbeginInfo.setClearValues(clearValues);
	renderPassbeginInfo.setFramebuffer(framebuffers[currentFrame]);
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
	FrameData& frameData = DGame->render.frameDatas[currentFrame];

	std::vector<vk::DescriptorSet> descriptorSets = {
		*frameData.descriptorSet,
		scene->gltfDescriptor
	};

	commandBuffer.beginRenderPass(renderPassbeginInfo, vk::SubpassContents::eInline);
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline);
	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipeline->GetLayout(), 0, descriptorSets,nullptr);
	//commandBuffer.draw(3, 1, 0, 0);
	
	for (auto& rootNode : scene->GetRootNodes()) {
		auto node = rootNode;
		node->Draw(commandBuffer);

	}


	commandBuffer.endRenderPass(); 

	depthImageGUI[currentFrame].layout = vk::ImageLayout::eColorAttachmentOptimal;
	depthImageGUI[currentFrame].setImageLayout(commandBuffer, vk::ImageLayout::eShaderReadOnlyOptimal);


}

void DDing::ForwardPass::DrawUI()
{
	int currentFrame = DGame->render.currentFrame;
	if (ImGui::CollapsingHeader("ForwardPass")) {
		ImGui::Text("Depth Image");
		ImGui::Image((ImTextureID)depthImageDescriptorSet[currentFrame], ImVec2(DGame->swapChain.extent.width * 0.2, DGame->swapChain.extent.height * 0.2));
		
	}
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
	imageInfo.setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled);
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

void DDing::RenderPass::createOutputImages()
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
		std::array<vk::ImageView, 3> attachments = {
			outputImages[i].imageView,
			depthImageGUI[i].imageView,
			depthImage.imageView
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

void DDing::ForwardPass::initDepthImageGUI()
{

	vk::SamplerCreateInfo samplerInfo{};
	samplerInfo.setMagFilter(vk::Filter::eLinear);
	samplerInfo.setMinFilter(vk::Filter::eLinear);
	samplerInfo.setMipmapMode(vk::SamplerMipmapMode::eLinear);
	samplerInfo.setAddressModeU(vk::SamplerAddressMode::eRepeat);
	samplerInfo.setAddressModeV(vk::SamplerAddressMode::eRepeat);
	samplerInfo.setAddressModeW(vk::SamplerAddressMode::eRepeat);
	samplerInfo.setMinLod(-1000);
	samplerInfo.setMaxLod(1000);
	samplerInfo.setMaxAnisotropy(1.0f);
	depthImageSampler = vk::raii::Sampler(DGame->context.logical, samplerInfo);


	for (int i = 0; i < FRAME_CNT; i++) {
		vk::ImageCreateInfo imageInfo{};
		imageInfo.setArrayLayers(1);
		imageInfo.setExtent({ DGame->swapChain.extent.width, DGame->swapChain.extent.height, 1 });
		imageInfo.setFormat(DDing::ForwardPass::ColorFormat);
		imageInfo.setImageType(vk::ImageType::e2D);
		imageInfo.setMipLevels(1);
		imageInfo.setSamples(vk::SampleCountFlagBits::e1);
		imageInfo.setTiling(vk::ImageTiling::eOptimal);
		imageInfo.setUsage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled);
		imageInfo.setInitialLayout(vk::ImageLayout::eUndefined);
		imageInfo.setSharingMode(vk::SharingMode::eExclusive);

		vk::ImageViewCreateInfo imageViewInfo{};
		imageViewInfo.setFormat(imageInfo.format);
		imageViewInfo.setViewType(vk::ImageViewType::e2D);
		imageViewInfo.setSubresourceRange({ vk::ImageAspectFlagBits::eColor, 0, imageInfo.mipLevels, 0, 1 });

		VmaAllocationCreateInfo allocInfo  = {};
		allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
		allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
		allocInfo.priority = 1.0f;


		depthImageGUI.push_back(DDing::Image(imageInfo, allocInfo, imageViewInfo));
		DGame->context.immediate_submit([&](vk::CommandBuffer commandBuffer) {
			depthImageGUI[i].setImageLayout(commandBuffer, vk::ImageLayout::eShaderReadOnlyOptimal);
			});


		depthImageDescriptorSet.push_back(ImGui_ImplVulkan_AddTexture(*depthImageSampler, depthImageGUI[i].imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));

	}




	
}

DDing::DeferredPass::DeferredPass(Pipeline& pipeline, vk::RenderPass renderPass)
	:RenderPass(pipeline, renderPass)
{

}

void DDing::DeferredPass::Render(vk::CommandBuffer commandBuffer, DDing::Scene* scene)
{

}

DDing::Image& DDing::RenderPass::GetOutputImage()
{
	return outputImages[DGame->render.currentFrame];
}

DDing::ShadowPass::ShadowPass(Pipeline& pipeline, vk::RenderPass renderPass)
	: RenderPass(pipeline, renderPass)
{
}

void DDing::ShadowPass::Render(vk::CommandBuffer commandBuffer, DDing::Scene* scene)
{
	createOutputImages();
}

void DDing::ShadowPass::createOutputImages()
{
	for (int i = 0; i < MAX_LIGHTS; i++) {
		
		//For PointLight
		{
			
			vk::ImageCreateInfo imageInfo{};
			imageInfo.setImageType(vk::ImageType::e2D);
			imageInfo.setExtent(vk::Extent3D{ DGame->swapChain.extent,1 });
			imageInfo.setMipLevels(1);
			imageInfo.setArrayLayers(6);
			imageInfo.setTiling(vk::ImageTiling::eOptimal);
			imageInfo.setInitialLayout(vk::ImageLayout::eUndefined);
			imageInfo.setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment);
			imageInfo.setSamples(vk::SampleCountFlagBits::e1);
			imageInfo.setSharingMode(vk::SharingMode::eExclusive);
			imageInfo.setFlags(vk::ImageCreateFlagBits::eCubeCompatible);
			imageInfo.setFormat(vk::Format::eD32Sfloat);

			VmaAllocationCreateInfo allocInfo{};
			allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
			allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
			allocInfo.priority = 1.0f;
			
			vk::ImageViewCreateInfo imageViewInfo{};
			imageViewInfo.setFormat(vk::Format::eD32Sfloat);
			imageViewInfo.setSubresourceRange(vk::ImageSubresourceRange{ vk::ImageAspectFlagBits::eDepth,0,1,0,6 });
			imageViewInfo.setViewType(vk::ImageViewType::eCube);

			outputImages.push_back(DDing::Image(imageInfo, allocInfo, imageViewInfo));

			
		}
		//TODO, For Directional, SpotLight
	}
}


void DDing::ShadowPass::createFramebuffers()
{
	for (int i = 0; i < FRAME_CNT; i++) {

	}
}
