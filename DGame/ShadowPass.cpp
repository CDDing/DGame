#include "pch.h"
#include "ShadowPass.h"
#include "Light.h"
#include "Transform.h"
vk::Format DDing::ShadowPass::DepthFormat = vk::Format::eD32Sfloat;

DDing::ShadowPass::ShadowPass()
{
	//createOutputImages();
	//createPointLightShadowMapViews();

	////createDescriptors();
	//createFramebuffers();
}

DDing::ShadowPass::~ShadowPass()
{
}

void DDing::ShadowPass::InitRenderPass()
{
	vk::AttachmentDescription depthAttachment{};
	depthAttachment.format = DDing::ShadowPass::DepthFormat;
	depthAttachment.samples = vk::SampleCountFlagBits::e1;
	depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	depthAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	depthAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	depthAttachment.initialLayout = vk::ImageLayout::eUndefined;
	depthAttachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;


	vk::AttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment = 0;
	depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;


	vk::SubpassDescription subpass{};
	subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	subpass.setColorAttachments({});
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	vk::SubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
	dependency.srcAccessMask = {};
	dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
	dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

	std::vector<vk::AttachmentDescription> attachments = { depthAttachment };
	vk::RenderPassCreateInfo renderPassInfo{};
	renderPassInfo.setAttachments(attachments);
	renderPassInfo.setSubpasses(subpass);
	renderPassInfo.setDependencies(dependency);
	renderPass = DGame->context.logical.createRenderPass(renderPassInfo);

}

void DDing::ShadowPass::InitPipeline()
{
	PipelineDesc pipelineDesc{};

	auto vertShaderCode = loadShader("Shaders/DepthOnly.vert.spv");
	vk::ShaderModuleCreateInfo vertCreateInfo{};
	vertCreateInfo.setCode(vertShaderCode);
	vk::raii::ShaderModule vertShaderModule = DGame->context.logical.createShaderModule(vertCreateInfo);
	vk::PipelineShaderStageCreateInfo vertStage{};
	vertStage.setModule(*vertShaderModule);
	vertStage.setPName("main");
	vertStage.setStage(vk::ShaderStageFlagBits::eVertex);

	//auto fragShaderCode = loadShader("Shaders/DepthOnly.frag.spv");
	//vk::ShaderModuleCreateInfo fragCreateInfo{};
	//fragCreateInfo.setCode(fragShaderCode);
	//vk::raii::ShaderModule fragShaderModule = DGame->context.logical.createShaderModule(fragCreateInfo);
	//vk::PipelineShaderStageCreateInfo fragStage{};
	//fragStage.setModule(*fragShaderModule);
	//fragStage.setPName("main");
	//fragStage.setStage(vk::ShaderStageFlagBits::eFragment);


	std::vector<vk::PipelineShaderStageCreateInfo> shaderStages = { vertStage };
	pipelineDesc.shaderStages = shaderStages;


	std::vector<vk::DynamicState> dynamicStates;
	vk::PipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.setDynamicStates(dynamicStates);
	pipelineDesc.dynamicState = dynamicState;

	//TODO
	vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.setVertexAttributeDescriptions({});
	vertexInputInfo.setVertexBindingDescriptions({});
	pipelineDesc.vertexInput = vertexInputInfo;

	vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.setPrimitiveRestartEnable(vk::False);
	inputAssembly.setTopology(vk::PrimitiveTopology::eTriangleList);
	pipelineDesc.inputAssembly = inputAssembly;

	vk::Viewport viewport{};
	viewport.setWidth(DGame->swapChain.extent.width);
	viewport.setHeight(DGame->swapChain.extent.height);
	viewport.setX(0.0f);
	viewport.setY(0.0f);
	viewport.setMinDepth(0.0f);
	viewport.setMaxDepth(1.0f);

	vk::Rect2D scissor{};
	scissor.setExtent(DGame->swapChain.extent);

	vk::PipelineViewportStateCreateInfo viewportState{};
	viewportState.setScissors(scissor);
	viewportState.setViewports(viewport);
	pipelineDesc.viewportState = viewportState;

	vk::PipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.setCullMode(vk::CullModeFlagBits::eFront);
	rasterizer.setFrontFace(vk::FrontFace::eCounterClockwise);
	rasterizer.setPolygonMode(vk::PolygonMode::eFill);
	rasterizer.setDepthClampEnable(vk::False);
	rasterizer.setRasterizerDiscardEnable(vk::False);
	rasterizer.setLineWidth(1.0f);
	rasterizer.setDepthBiasEnable(vk::False);
	rasterizer.setDepthBiasConstantFactor(0.0f);
	rasterizer.setDepthBiasClamp(0.0f);
	rasterizer.setDepthBiasSlopeFactor(0.0f);
	pipelineDesc.rasterizer = rasterizer;

	vk::PipelineMultisampleStateCreateInfo multiSampling{};
	multiSampling.setRasterizationSamples(vk::SampleCountFlagBits::e1);
	multiSampling.setSampleShadingEnable(vk::False);
	multiSampling.setMinSampleShading(1.0f);
	multiSampling.setAlphaToCoverageEnable(vk::False);
	multiSampling.setAlphaToOneEnable(vk::False);
	pipelineDesc.multiSample = multiSampling;




	vk::PipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.setLogicOpEnable(vk::False);
	colorBlending.setLogicOp(vk::LogicOp::eCopy);
	colorBlending.setAttachments({});
	colorBlending.setBlendConstants({ 0 });
	pipelineDesc.colorBlend = colorBlending;

	vk::PipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.setDepthTestEnable(vk::True);
	depthStencil.setDepthWriteEnable(vk::True);
	depthStencil.setDepthCompareOp(vk::CompareOp::eLess);
	depthStencil.setDepthBoundsTestEnable(vk::False);
	depthStencil.setMinDepthBounds(0.0f);
	depthStencil.setMaxDepthBounds(1.0f);
	depthStencil.setStencilTestEnable(vk::False);
	depthStencil.setFront(vk::StencilOpState{});
	depthStencil.setBack(vk::StencilOpState{});
	pipelineDesc.depthStencil = depthStencil;

	//TODO
	//std::vector<vk::DescriptorSetLayout> setLayouts = { *globalSetLayout };
	//vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
	//pipelineLayoutInfo.setSetLayouts(setLayouts);
	//vk::PushConstantRange pushConstantRange{};
	//pushConstantRange.setOffset(0);
	//pushConstantRange.setSize(sizeof(DDing::ForwardPass::PushConstant));
	//pushConstantRange.setStageFlags(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);
	//pipelineLayoutInfo.setPushConstantRanges({ pushConstantRange });
	//pipelineDesc.layout = pipelineLayoutInfo;

	//TODO
	pipelineDesc.renderPass = *renderPass;


	auto pipeline = std::make_unique<DDing::GraphicsPipeline>(DGame->context, pipelineDesc);
	
}

void DDing::ShadowPass::InitFrameData()
{
}

void DDing::ShadowPass::Render(vk::CommandBuffer commandBuffer)
{


	auto currentFrame = DGame->render.currentFrame;
	auto currentScene = DGame->scene.currentScene;
	glm::mat4 viewMatrix = glm::mat4(1.0f);

	vk::Viewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(DGame->swapChain.extent.height);
	viewport.height = static_cast<float>(DGame->swapChain.extent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	vk::Rect2D scissor{};
	scissor.offset = vk::Offset2D(0, 0);
	scissor.extent = DGame->swapChain.extent;

	commandBuffer.setViewport(0, viewport);
	commandBuffer.setScissor(0, scissor);

	int lightCnt = 0;
	for (auto& node : currentScene->GetNodes()) {
		auto transform = node->GetComponent<DDing::Transform>();
		auto lightComponent = node->GetComponent<DDing::Light>();
		if (lightComponent) {
			//TODO for supporting Light Types
			if (lightComponent->type == LightType::ePoint) {

			}

			//Now For PointLights
			for (int faceIndex = 0; faceIndex < 6; faceIndex) {

				auto idx = GetPointLightIndex(currentFrame, lightCnt, faceIndex);

				vk::ClearValue clearValues[2];

				vk::RenderPassBeginInfo renderPassBeginInfo{};
				renderPassBeginInfo.setRenderPass(renderPass);
				renderPassBeginInfo.setRenderArea(vk::Rect2D());
				renderPassBeginInfo.setFramebuffer(*pointLightFramebuffers[idx]);

				switch (faceIndex)
				{
				case 0: // POSITIVE_X
					viewMatrix = glm::lookAtLH(transform->GetWorldPosition(), transform->GetWorldPosition() + glm::vec3(1, 0, 0), glm::vec3(0, 1, 0));
					break;
				case 1:	// NEGATIVE_X
					viewMatrix = glm::lookAtLH(transform->GetWorldPosition(), transform->GetWorldPosition() + glm::vec3(-1, 0, 0), glm::vec3(0, 1, 0));
					break;
				case 2:	// POSITIVE_Y
					viewMatrix = glm::lookAtLH(transform->GetWorldPosition(), transform->GetWorldPosition() + glm::vec3(0, 1, 0), glm::vec3(0, 1, 0));
					break;
				case 3:	// NEGATIVE_Y
					viewMatrix = glm::lookAtLH(transform->GetWorldPosition(), transform->GetWorldPosition() + glm::vec3(0, -1, 0), glm::vec3(0, 1, 0));
					break;
				case 4:	// POSITIVE_Z
					viewMatrix = glm::lookAtLH(transform->GetWorldPosition(), transform->GetWorldPosition() + glm::vec3(0, 0, 1), glm::vec3(0, 1, 0));
					break;
				case 5:	// NEGATIVE_Z
					viewMatrix = glm::lookAtLH(transform->GetWorldPosition(), transform->GetWorldPosition() + glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));
					break;
				}

				commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

				commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline);

				std::vector<vk::DescriptorSet> descriptorSetList{ *descriptorSets[currentFrame] };
				commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline->GetLayout(), 0, descriptorSetList, {});

				for (auto& node : currentScene->GetNodes()) {
					node->Draw(commandBuffer, *pipeline->GetLayout());
				}

				commandBuffer.endRenderPass();

			}

			lightCnt++;

		}
	}

}

void DDing::ShadowPass::DrawUI()
{
}

void DDing::ShadowPass::createOutputImages()
{
	for (int frameCnt = 0; frameCnt < FRAME_CNT; frameCnt++) {
		//push by order of enum LightType

		//TODO
		//For Directional
		for (int i = 0; i < MAX_LIGHTS; i++) {



			vk::ImageCreateInfo imageInfo{};
			imageInfo.setImageType(vk::ImageType::e2D);
			imageInfo.setExtent(vk::Extent3D{ DGame->swapChain.extent.height,DGame->swapChain.extent.height,1 });
			imageInfo.setMipLevels(1);
			imageInfo.setArrayLayers(6);
			imageInfo.setTiling(vk::ImageTiling::eOptimal);
			imageInfo.setInitialLayout(vk::ImageLayout::eUndefined);
			imageInfo.setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment);
			imageInfo.setSamples(vk::SampleCountFlagBits::e1);
			imageInfo.setSharingMode(vk::SharingMode::eExclusive);
			imageInfo.setFlags(vk::ImageCreateFlagBits::eCubeCompatible);
			imageInfo.setFormat(DepthFormat);

			VmaAllocationCreateInfo allocInfo{};
			allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
			allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
			allocInfo.priority = 1.0f;

			//Dummy Image View
			vk::ImageViewCreateInfo imageViewInfo{};
			imageViewInfo.setFormat(DepthFormat);
			imageViewInfo.setSubresourceRange(vk::ImageSubresourceRange{ vk::ImageAspectFlagBits::eDepth,0,1,0,6 });
			imageViewInfo.setViewType(vk::ImageViewType::eCube);

			outputImages.push_back(DDing::Image(imageInfo, allocInfo, imageViewInfo));



		}
		//For PointLight
		for (int i = 0; i < MAX_LIGHTS; i++) {



			vk::ImageCreateInfo imageInfo{};
			imageInfo.setImageType(vk::ImageType::e2D);
			imageInfo.setExtent(vk::Extent3D{ DGame->swapChain.extent.height,DGame->swapChain.extent.height,1 });
			imageInfo.setMipLevels(1);
			imageInfo.setArrayLayers(6);
			imageInfo.setTiling(vk::ImageTiling::eOptimal);
			imageInfo.setInitialLayout(vk::ImageLayout::eUndefined);
			imageInfo.setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment);
			imageInfo.setSamples(vk::SampleCountFlagBits::e1);
			imageInfo.setSharingMode(vk::SharingMode::eExclusive);
			imageInfo.setFlags(vk::ImageCreateFlagBits::eCubeCompatible);
			imageInfo.setFormat(DepthFormat);

			VmaAllocationCreateInfo allocInfo{};
			allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
			allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
			allocInfo.priority = 1.0f;

			//Dummy Image View
			vk::ImageViewCreateInfo imageViewInfo{};
			imageViewInfo.setFormat(DepthFormat);
			imageViewInfo.setSubresourceRange(vk::ImageSubresourceRange{ vk::ImageAspectFlagBits::eDepth,0,1,0,6 });
			imageViewInfo.setViewType(vk::ImageViewType::eCube);

			outputImages.push_back(DDing::Image(imageInfo, allocInfo, imageViewInfo));



		}
		//TODO
		//For SpotLight
		for (int i = 0; i < MAX_LIGHTS; i++) {

			vk::ImageCreateInfo imageInfo{};
			imageInfo.setImageType(vk::ImageType::e2D);
			imageInfo.setExtent(vk::Extent3D{ DGame->swapChain.extent.height,DGame->swapChain.extent.height,1 });
			imageInfo.setMipLevels(1);
			imageInfo.setArrayLayers(6);
			imageInfo.setTiling(vk::ImageTiling::eOptimal);
			imageInfo.setInitialLayout(vk::ImageLayout::eUndefined);
			imageInfo.setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment);
			imageInfo.setSamples(vk::SampleCountFlagBits::e1);
			imageInfo.setSharingMode(vk::SharingMode::eExclusive);
			imageInfo.setFlags(vk::ImageCreateFlagBits::eCubeCompatible);
			imageInfo.setFormat(DepthFormat);

			VmaAllocationCreateInfo allocInfo{};
			allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
			allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
			allocInfo.priority = 1.0f;

			//Dummy Image View
			vk::ImageViewCreateInfo imageViewInfo{};
			imageViewInfo.setFormat(DepthFormat);
			imageViewInfo.setSubresourceRange(vk::ImageSubresourceRange{ vk::ImageAspectFlagBits::eDepth,0,1,0,6 });
			imageViewInfo.setViewType(vk::ImageViewType::eCube);

			outputImages.push_back(DDing::Image(imageInfo, allocInfo, imageViewInfo));



		}
	}
}


void DDing::ShadowPass::createPointLightShadowMapViews()
{
	for (int frameCnt = 0; frameCnt < FRAME_CNT; frameCnt++) {
		for (int i = 0; i < MAX_LIGHTS; i++) {


			for (int j = 0; j < 6; j++) {
				vk::ImageViewCreateInfo imageViewInfo{};
				imageViewInfo.setViewType(vk::ImageViewType::e2D);
				imageViewInfo.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth, 0, 1, i, 1));
				imageViewInfo.setFormat(DepthFormat);
				imageViewInfo.setImage(outputImages[frameCnt * MAX_LIGHTS + i].image);

				pointLightShadowMapViews.push_back(vk::raii::ImageView(DGame->context.logical, imageViewInfo));
			}
		}
	}

}

void DDing::ShadowPass::createDescriptors()
{
	{
		vk::DescriptorPoolSize poolSize{};
		poolSize.setType(vk::DescriptorType::eUniformBuffer);
		poolSize.setDescriptorCount(FRAME_CNT);

		vk::DescriptorPoolCreateInfo poolInfo{};
		poolInfo.setMaxSets(FRAME_CNT);
		poolInfo.setPoolSizes(poolSize);
		poolInfo.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);

		descriptorPool = DGame->context.logical.createDescriptorPool(poolInfo);
	}
	{
		vk::DescriptorSetAllocateInfo allocInfo{};
		allocInfo.setDescriptorPool(*descriptorPool);
		allocInfo.setDescriptorSetCount(FRAME_CNT);
		//allocInfo.setSetLayouts(*DGame->render.globalSetLayout);

		//descriptorSets = DGame->context.logical.allocateDescriptorSets(allocInfo);
	}
	for (int i = 0; i < FRAME_CNT; i++) {
		{
			vk::BufferCreateInfo bufferInfo{};
			bufferInfo.setSize(sizeof(GlobalBuffer));
			bufferInfo.setUsage(vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst);

			VmaAllocationCreateInfo allocInfo{};
			allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
			allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
			allocInfo.priority = 1.0f;

			auto uniformBuffer = DDing::Buffer(bufferInfo, allocInfo);
			uniformBuffers.push_back(std::move(uniformBuffer));
		}
		{
			vk::BufferCreateInfo stagingInfo{ };
			stagingInfo.setSize(sizeof(GlobalBuffer));
			stagingInfo.setUsage(vk::BufferUsageFlagBits::eTransferSrc);

			VmaAllocationCreateInfo allocInfo{};
			allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
			allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

			auto stagingBuffer = DDing::Buffer(stagingInfo, allocInfo);
			stagingBuffers.push_back(std::move(stagingBuffer));
		}
		{
			vk::DescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = uniformBuffers[i].buffer;
			bufferInfo.range = sizeof(GlobalBuffer);
			bufferInfo.offset = 0;

			vk::WriteDescriptorSet descriptorWrite{};
			descriptorWrite.dstSet = *descriptorSets[i];
			descriptorWrite.dstBinding = 0;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = vk::DescriptorType::eUniformBuffer;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;

			DGame->context.logical.updateDescriptorSets(descriptorWrite, nullptr);
		}
	}
}

void DDing::ShadowPass::createFramebuffers()
{
	for (int frameCnt = 0; frameCnt < FRAME_CNT; frameCnt++) {
		for (int i = 0; i < MAX_LIGHTS; i++) {
			//Point Light
			{
				for (int j = 0; j < 6; j++) {

					auto idx = GetPointLightIndex(frameCnt, i, j);
					std::array<vk::ImageView, 1> attachments = {
						*pointLightShadowMapViews[idx],
					};
					vk::FramebufferCreateInfo framebufferInfo{};
					framebufferInfo.setRenderPass(renderPass);
					framebufferInfo.setAttachments(attachments);
					framebufferInfo.setWidth(DGame->swapChain.extent.height);
					framebufferInfo.setHeight(DGame->swapChain.extent.height);
					framebufferInfo.setLayers(1);

					pointLightFramebuffers.emplace_back(DGame->context.logical, framebufferInfo);

				}
			}

			//Directional Light
			{

			}

			//Spot Light
			{

			}
		}
	}
}

uint32_t DDing::ShadowPass::GetPointLightIndex(int frameCnt, int lightCnt, int faceIndex)
{
	return frameCnt * MAX_LIGHTS * 6 + lightCnt * 6 + faceIndex;
}
