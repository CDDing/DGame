#include "pch.h"
#include "ShadowPass.h"
#include "Light.h"
#include "Transform.h"
vk::Format DDing::ShadowPass::DepthFormat = vk::Format::eD32Sfloat;
vk::Format DDing::ShadowPass::SampleFormat = vk::Format::eR8G8B8A8Unorm;

DDing::ShadowPass::ShadowPass()
{
	InitDescriptors();
	InitFrameData();
	InitRenderPass();
	InitPipeline();
	createOutputImages();
	createDepthImage();
	createPointLightShadowMapViews();

	createFramebuffers();
}

DDing::ShadowPass::~ShadowPass()
{
}

void DDing::ShadowPass::InitRenderPass()
{
	vk::AttachmentDescription colorAttachment{};
	colorAttachment.format = DDing::ShadowPass::SampleFormat;
	colorAttachment.samples = vk::SampleCountFlagBits::e1;
	colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
	colorAttachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;


	vk::AttachmentDescription depthAttachment{};
	depthAttachment.format = DDing::ShadowPass::DepthFormat;
	depthAttachment.samples = vk::SampleCountFlagBits::e1;
	depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	depthAttachment.storeOp = vk::AttachmentStoreOp::eDontCare;
	depthAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	depthAttachment.initialLayout = vk::ImageLayout::eUndefined;
	depthAttachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

	vk::AttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

	vk::AttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

	std::vector<vk::AttachmentReference> colorAttachments{ colorAttachmentRef };

	vk::SubpassDescription subpass{};
	subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	subpass.setColorAttachments(colorAttachments);
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	vk::SubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
	dependency.srcAccessMask = {};
	dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
	dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

	std::vector<vk::AttachmentDescription> attachments = { colorAttachment, depthAttachment };
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

	auto fragShaderCode = loadShader("Shaders/DepthOnly.frag.spv");
	vk::ShaderModuleCreateInfo fragCreateInfo{};
	fragCreateInfo.setCode(fragShaderCode);
	vk::raii::ShaderModule fragShaderModule = DGame->context.logical.createShaderModule(fragCreateInfo);
	vk::PipelineShaderStageCreateInfo fragStage{};
	fragStage.setModule(*fragShaderModule);
	fragStage.setPName("main");
	fragStage.setStage(vk::ShaderStageFlagBits::eFragment);


	std::vector<vk::PipelineShaderStageCreateInfo> shaderStages = { vertStage, fragStage };
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
	viewport.setWidth(GetLength());
	viewport.setHeight(GetLength());
	viewport.setX(0.0f);
	viewport.setY(0.0f);
	viewport.setMinDepth(0.0f);
	viewport.setMaxDepth(1.0f);

	vk::Rect2D scissor{};
	scissor.setExtent({ GetLength() ,GetLength() });

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




	vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
	colorBlendAttachment.setBlendEnable(vk::False);
	colorBlendAttachment.setSrcColorBlendFactor(vk::BlendFactor::eOne);
	colorBlendAttachment.setDstColorBlendFactor(vk::BlendFactor::eZero);
	colorBlendAttachment.setColorBlendOp(vk::BlendOp::eAdd);
	colorBlendAttachment.setSrcAlphaBlendFactor(vk::BlendFactor::eOne);
	colorBlendAttachment.setDstAlphaBlendFactor(vk::BlendFactor::eZero);
	colorBlendAttachment.setAlphaBlendOp(vk::BlendOp::eAdd);

	
	std::vector<vk::PipelineColorBlendAttachmentState> attachments{ colorBlendAttachment };

	vk::PipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.setLogicOpEnable(vk::False);
	colorBlending.setLogicOp(vk::LogicOp::eCopy);
	colorBlending.setAttachments(attachments);
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
	std::vector<vk::DescriptorSetLayout> setLayouts = { *sceneSetLayout };
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.setSetLayouts(setLayouts);
	vk::PushConstantRange pushConstantRange{};
	pushConstantRange.setOffset(0);
	pushConstantRange.setSize(sizeof(DrawPushConstant));
	pushConstantRange.setStageFlags(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);
	pipelineLayoutInfo.setPushConstantRanges({ pushConstantRange });
	pipelineDesc.layout = pipelineLayoutInfo;

	//TODO
	pipelineDesc.renderPass = *renderPass;


	pipeline = std::make_unique<DDing::GraphicsPipeline>(DGame->context, pipelineDesc);

}

void DDing::ShadowPass::InitFrameData()
{

	for (int i = 0; i < FRAME_CNT; i++) {
		ShadowFrameData frameData{};
		{

			std::vector<vk::DescriptorPoolSize> poolSizes = {
				vk::DescriptorPoolSize(vk::DescriptorType::eUniformBufferDynamic,1),
			};

			vk::DescriptorPoolCreateInfo poolInfo{};
			poolInfo.setMaxSets(FRAME_CNT);
			poolInfo.setPoolSizes(poolSizes);
			poolInfo.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet | vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind);

			frameData.descriptorPool = DGame->context.logical.createDescriptorPool(poolInfo);
		}
		{
			vk::DescriptorSetAllocateInfo allocInfo{};
			allocInfo.setDescriptorPool(*frameData.descriptorPool);
			allocInfo.setDescriptorSetCount(1);
			allocInfo.setSetLayouts(*sceneSetLayout);

			frameData.descriptorSet = std::move(DGame->context.logical.allocateDescriptorSets(allocInfo).front());
		}
		{
			vk::BufferCreateInfo bufferInfo{};
			bufferInfo.setSize(sizeof(TotalShadowBuffer));
			bufferInfo.setUsage(vk::BufferUsageFlagBits::eUniformBuffer| vk::BufferUsageFlagBits::eTransferDst);

			VmaAllocationCreateInfo allocInfo{};
			allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
			allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
			allocInfo.priority = 1.0f;

			frameData.uniformBuffer = DDing::Buffer(bufferInfo, allocInfo);

		}
		{
			vk::BufferCreateInfo stagingInfo{ };
			stagingInfo.setSize(sizeof(TotalShadowBuffer));
			stagingInfo.setUsage(vk::BufferUsageFlagBits::eTransferSrc);

			VmaAllocationCreateInfo allocInfo{};
			allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
			allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

			frameData.stagingBuffer = DDing::Buffer(stagingInfo, allocInfo);
		}
		{
			vk::DescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = frameData.uniformBuffer.buffer;
			bufferInfo.range = sizeof(ShadowBuffer);
			bufferInfo.offset = 0;

			vk::WriteDescriptorSet descriptorWrite{};
			descriptorWrite.dstSet = *frameData.descriptorSet;
			descriptorWrite.dstBinding = 0;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = vk::DescriptorType::eUniformBufferDynamic;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;

			DGame->context.logical.updateDescriptorSets(descriptorWrite, nullptr);
		}
		frameDatas.push_back(std::move(frameData));
	}
}

void DDing::ShadowPass::InitDescriptors()
{
	{
		vk::DescriptorSetLayoutBinding layoutBinding{};
		layoutBinding.binding = 0;
		layoutBinding.descriptorType = vk::DescriptorType::eUniformBufferDynamic;
		layoutBinding.descriptorCount = 1;
		layoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment;

		vk::DescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.setBindings(layoutBinding);
		sceneSetLayout = DGame->context.logical.createDescriptorSetLayout(layoutInfo);
	}
}

void DDing::ShadowPass::Render(vk::CommandBuffer commandBuffer)
{


	auto currentFrame = DGame->render.currentFrame;
	auto currentScene = DGame->scene.currentScene;

	auto& frameData = frameDatas[currentFrame];

	SetBuffer(commandBuffer);
	glm::mat4 viewMatrix = glm::mat4(1.0f);

	vk::Viewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(GetLength());
	viewport.height = static_cast<float>(GetLength());
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	vk::Rect2D scissor{};
	scissor.offset = vk::Offset2D(0, 0);
	scissor.extent = vk::Extent2D{ GetLength(),GetLength() };

	commandBuffer.setViewport(0, viewport);
	commandBuffer.setScissor(0, scissor);

	int dCnt = 0, pCnt = 0, sCnt = 0; //Directional, Point, Spot
	for (auto& node : currentScene->GetNodes()) {
		auto transform = node->GetComponent<DDing::Transform>();
		auto lightComponent = node->GetComponent<DDing::Light>();
		if (lightComponent) {
			switch (lightComponent->type) {
			case LightType::eDirectional:
				//TODO

				dCnt++;
				break;
			case LightType::ePoint:

				for (int faceIndex = 0; faceIndex < 6; faceIndex++) {

					auto idx = GetPointLightIndex(currentFrame, pCnt, faceIndex);

					vk::ClearValue clearValues[2];
					clearValues[0].color = vk::ClearColorValue{ 1.0f, 1.0f, 1.0f, 1.0f };
					clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);

					vk::RenderPassBeginInfo renderPassBeginInfo{};
					renderPassBeginInfo.setRenderPass(renderPass);
					renderPassBeginInfo.setRenderArea(vk::Rect2D(vk::Offset2D(0, 0), vk::Extent2D(GetLength(),GetLength())));
					renderPassBeginInfo.setFramebuffer(*pointLightFramebuffers[idx]);
					renderPassBeginInfo.setClearValues(clearValues);

					commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

					commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline);

					uint32_t dynamicOffset = offsetof(TotalShadowBuffer, point) + sizeof(ShadowBuffer) * faceIndex;
					std::vector<vk::DescriptorSet> descriptorSetList{ *frameData.descriptorSet };
					commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline->GetLayout(), 0, descriptorSetList, {dynamicOffset});
					for (auto& rootNode : currentScene->GetRootNodes()) {
						auto node = rootNode;
						node->Draw(commandBuffer, *pipeline->GetLayout());

					}


					commandBuffer.endRenderPass();

				}

				pCnt++;
				break;
			case LightType::eSpot:
				//TODO

				sCnt++;
				break;
			}

		}
	}

}

void DDing::ShadowPass::DrawUI()
{
	int currentFrame = DGame->render.currentFrame;
	if (ImGui::CollapsingHeader("ShadowPass")) {


		
	}
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
			imageInfo.setExtent(vk::Extent3D{ GetLength(),GetLength(),1 });
			imageInfo.setMipLevels(1);
			imageInfo.setArrayLayers(1);
			imageInfo.setTiling(vk::ImageTiling::eOptimal);
			imageInfo.setInitialLayout(vk::ImageLayout::eUndefined);
			imageInfo.setUsage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled);
			imageInfo.setSamples(vk::SampleCountFlagBits::e1);
			imageInfo.setSharingMode(vk::SharingMode::eExclusive);
			imageInfo.setFormat(SampleFormat);

			VmaAllocationCreateInfo allocInfo{};
			allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
			allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
			allocInfo.priority = 1.0f;

			//Dummy Image View
			vk::ImageViewCreateInfo imageViewInfo{};
			imageViewInfo.setFormat(SampleFormat);
			imageViewInfo.setSubresourceRange(vk::ImageSubresourceRange{ vk::ImageAspectFlagBits::eColor,0,1,0,1 });
			imageViewInfo.setViewType(vk::ImageViewType::e2D);

			outputImages.push_back(DDing::Image(imageInfo, allocInfo, imageViewInfo));



		}
		//For PointLight
		for (int i = 0; i < MAX_LIGHTS; i++) {



			vk::ImageCreateInfo imageInfo{};
			imageInfo.setImageType(vk::ImageType::e2D);
			imageInfo.setExtent(vk::Extent3D{ GetLength(),GetLength(),1 });
			imageInfo.setMipLevels(1);
			imageInfo.setArrayLayers(6);
			imageInfo.setTiling(vk::ImageTiling::eOptimal);
			imageInfo.setInitialLayout(vk::ImageLayout::eUndefined);
			imageInfo.setUsage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled);
			imageInfo.setSamples(vk::SampleCountFlagBits::e1);
			imageInfo.setSharingMode(vk::SharingMode::eExclusive);
			imageInfo.setFlags(vk::ImageCreateFlagBits::eCubeCompatible);
			imageInfo.setFormat(SampleFormat);

			VmaAllocationCreateInfo allocInfo{};
			allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
			allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
			allocInfo.priority = 1.0f;

			//Dummy Image View
			vk::ImageViewCreateInfo imageViewInfo{};
			imageViewInfo.setFormat(SampleFormat);
			imageViewInfo.setSubresourceRange(vk::ImageSubresourceRange{ vk::ImageAspectFlagBits::eColor,0,1,0,6 });
			imageViewInfo.setViewType(vk::ImageViewType::eCube);

			outputImages.push_back(DDing::Image(imageInfo, allocInfo, imageViewInfo));



		}
		//TODO
		//For SpotLight
		for (int i = 0; i < MAX_LIGHTS; i++) {

			vk::ImageCreateInfo imageInfo{};
			imageInfo.setImageType(vk::ImageType::e2D);
			imageInfo.setExtent(vk::Extent3D{ GetLength(),GetLength(),1 });
			imageInfo.setMipLevels(1);
			imageInfo.setArrayLayers(1);
			imageInfo.setTiling(vk::ImageTiling::eOptimal);
			imageInfo.setInitialLayout(vk::ImageLayout::eUndefined);
			imageInfo.setUsage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled);
			imageInfo.setSamples(vk::SampleCountFlagBits::e1);
			imageInfo.setSharingMode(vk::SharingMode::eExclusive);
			imageInfo.setFormat(SampleFormat);

			VmaAllocationCreateInfo allocInfo{};
			allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
			allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
			allocInfo.priority = 1.0f;

			//Dummy Image View
			vk::ImageViewCreateInfo imageViewInfo{};
			imageViewInfo.setFormat(SampleFormat);
			imageViewInfo.setSubresourceRange(vk::ImageSubresourceRange{ vk::ImageAspectFlagBits::eColor,0,1,0,1 });
			imageViewInfo.setViewType(vk::ImageViewType::e2D);

			outputImages.push_back(DDing::Image(imageInfo, allocInfo, imageViewInfo));



		}
	}
}


void DDing::ShadowPass::SetBuffer(vk::CommandBuffer commandBuffer)
{
	ShadowFrameData& frameData = frameDatas[DGame->render.currentFrame];

	void* mappedData = frameData.stagingBuffer.GetMappedPtr();

	directionalLightCnt = 0; pointLightCnt = 0; spotLightCnt = 0;
	for (auto& node : DGame->scene.currentScene->GetNodes()) {
		
		auto lightComponent = node->GetComponent<DDing::Light>();
		if (lightComponent) {
			ShadowBuffer point;
			auto transform = node->GetComponent<DDing::Transform>();

			switch (lightComponent->type) {
			case LightType::eDirectional:
				//TODO

				directionalLightCnt++;
				break;
			case LightType::ePoint:
				point.lightPosition = transform->GetWorldPosition();
				point.projection = glm::perspectiveFovLH(glm::radians(90.0f), static_cast<float>(DGame->swapChain.extent.width), static_cast<float>(DGame->swapChain.extent.height), 0.01f, 100.0f);
				
				
				auto viewMatrixPositiveX = glm::lookAtLH(transform->GetWorldPosition(), transform->GetWorldPosition() + glm::vec3(1, 0, 0), glm::vec3(0, 1, 0));
				point.view = viewMatrixPositiveX;
				memcpy((char*)mappedData + offsetof(TotalShadowBuffer, point) + pointLightCnt * sizeof(ShadowBuffer) * 6, &point, sizeof(ShadowBuffer));

				auto viewMatrixNegativeX = glm::lookAtLH(transform->GetWorldPosition(), transform->GetWorldPosition() + glm::vec3(-1, 0, 0), glm::vec3(0, 1, 0));
				point.view = viewMatrixNegativeX;
				memcpy((char*)mappedData + offsetof(TotalShadowBuffer, point) + pointLightCnt * sizeof(ShadowBuffer) * 6 + sizeof(ShadowBuffer) * 1, &point, sizeof(ShadowBuffer));

				auto viewMatrixPositiveY = glm::lookAtLH(transform->GetWorldPosition(), transform->GetWorldPosition() + glm::vec3(0, 1, 0), glm::vec3(0, 0, 1));
				point.view = viewMatrixPositiveY;
				memcpy((char*)mappedData + offsetof(TotalShadowBuffer, point) + pointLightCnt * sizeof(ShadowBuffer) * 6 + sizeof(ShadowBuffer) * 2, &point, sizeof(ShadowBuffer));

				auto viewMatrixNegativeY = glm::lookAtLH(transform->GetWorldPosition(), transform->GetWorldPosition() + glm::vec3(0, -1, 0), glm::vec3(0, 0, 1));
				point.view = viewMatrixNegativeY;
				memcpy((char*)mappedData + offsetof(TotalShadowBuffer, point) + pointLightCnt * sizeof(ShadowBuffer) * 6 + sizeof(ShadowBuffer) * 3, &point, sizeof(ShadowBuffer));

				auto viewMatrixPositiveZ = glm::lookAtLH(transform->GetWorldPosition(), transform->GetWorldPosition() + glm::vec3(0, 0, 1), glm::vec3(0, 1, 0));
				point.view = viewMatrixPositiveZ;
				memcpy((char*)mappedData + offsetof(TotalShadowBuffer, point) + pointLightCnt * sizeof(ShadowBuffer) * 6 + sizeof(ShadowBuffer) * 4, &point, sizeof(ShadowBuffer));

				auto viewMatrixNegativeZ = glm::lookAtLH(transform->GetWorldPosition(), transform->GetWorldPosition() + glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));
				point.view = viewMatrixNegativeZ;
				memcpy((char*)mappedData + offsetof(TotalShadowBuffer, point) + pointLightCnt * sizeof(ShadowBuffer) * 6 + sizeof(ShadowBuffer) * 5, &point, sizeof(ShadowBuffer));


				pointLightCnt++;
				break;
			case LightType::eSpot:
				//TODO

				spotLightCnt++;
				break;
			}
		}
	}



	VkBufferCopy copyRegion{};
	copyRegion.dstOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = sizeof(TotalShadowBuffer);

	vkCmdCopyBuffer(commandBuffer, frameData.stagingBuffer.buffer, frameData.uniformBuffer.buffer, 1, &copyRegion);
}


void DDing::ShadowPass::createDepthImage()
{
	vk::ImageCreateInfo imageInfo{};
	imageInfo.setArrayLayers(1);
	imageInfo.setExtent({ GetLength(), GetLength(), 1});
	imageInfo.setFormat(DDing::ShadowPass::DepthFormat);
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

void DDing::ShadowPass::createPointLightShadowMapViews()
{
	for (int frameCnt = 0; frameCnt < FRAME_CNT; frameCnt++) {
		for (int i = 0; i < MAX_LIGHTS; i++) {


			for (int j = 0; j < 6; j++) {
				vk::ImageViewCreateInfo imageViewInfo{};
				imageViewInfo.setViewType(vk::ImageViewType::e2D);
				imageViewInfo.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, i, 1));
				imageViewInfo.setFormat(SampleFormat);
				//3 means light types cnt
				imageViewInfo.setImage(outputImages[frameCnt * MAX_LIGHTS * 3 + MAX_LIGHTS + i ].image);

				pointLightShadowMapViews.push_back(vk::raii::ImageView(DGame->context.logical, imageViewInfo));
			}
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
					std::array<vk::ImageView, 2> attachments = {
						*pointLightShadowMapViews[idx],
						depthImage.imageView,
					};
					vk::FramebufferCreateInfo framebufferInfo{};
					framebufferInfo.setRenderPass(renderPass);
					framebufferInfo.setAttachments(attachments);
					framebufferInfo.setWidth(GetLength());
					framebufferInfo.setHeight(GetLength());
					framebufferInfo.setLayers(1);

					pointLightFramebuffers.emplace_back(DGame->context.logical, framebufferInfo);

				}

			}

			//Directional Light
			{
				std::array<vk::ImageView, 2> attachments = {
					outputImages[frameCnt * MAX_LIGHTS * 3 + i].imageView,
					depthImage.imageView,
				};
				vk::FramebufferCreateInfo framebufferInfo{};
				framebufferInfo.setRenderPass(renderPass);
				framebufferInfo.setAttachments(attachments);
				framebufferInfo.setWidth(GetLength());
				framebufferInfo.setHeight(GetLength());
				framebufferInfo.setLayers(1);

				directionalLightFramebuffers.emplace_back(DGame->context.logical, framebufferInfo);

			

			}

			//Spot Light
			{

				std::array<vk::ImageView, 2> attachments = {
					outputImages[frameCnt * MAX_LIGHTS * 3 + MAX_LIGHTS * 2 + i].imageView,
					depthImage.imageView
				};
				vk::FramebufferCreateInfo framebufferInfo{};
				framebufferInfo.setRenderPass(renderPass);
				framebufferInfo.setAttachments(attachments);
				framebufferInfo.setWidth(GetLength());
				framebufferInfo.setHeight(GetLength());
				framebufferInfo.setLayers(1);

				spotLightFramebuffers.emplace_back(DGame->context.logical, framebufferInfo);

		

			}
		}
	}
}

uint32_t DDing::ShadowPass::GetPointLightIndex(int frameCnt, int lightCnt, int faceIndex)
{
	return frameCnt * MAX_LIGHTS * 6 + lightCnt * 6 + faceIndex;
}
