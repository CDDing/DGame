#include "pch.h"
#include "ForwardPass.h"
#include "Camera.h"
#include "Light.h"
#include "Transform.h"

vk::Format DDing::ForwardPass::DepthFormat = vk::Format::eD32Sfloat;
vk::Format DDing::ForwardPass::ColorFormat = vk::Format::eB8G8R8A8Unorm;
DDing::ForwardPass::ForwardPass()
{
	InitDescriptors();
	InitFrameData();
	InitRenderPass();
	InitPipeline();



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

void DDing::ForwardPass::InitRenderPass()
{
	vk::AttachmentDescription colorAttachment{};
	colorAttachment.format = DDing::ForwardPass::ColorFormat;
	colorAttachment.samples = vk::SampleCountFlagBits::e1;
	colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
	colorAttachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

	vk::AttachmentDescription depthGUIAttachment{};
	depthGUIAttachment.format = DDing::ForwardPass::ColorFormat;
	depthGUIAttachment.samples = vk::SampleCountFlagBits::e1;
	depthGUIAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	depthGUIAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	depthGUIAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	depthGUIAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	depthGUIAttachment.initialLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	depthGUIAttachment.finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

	vk::AttachmentDescription depthAttachment{};
	depthAttachment.format = DDing::ForwardPass::DepthFormat;
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

	vk::AttachmentReference depthGUIAttachmentRef{};
	depthGUIAttachmentRef.attachment = 1;
	depthGUIAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

	vk::AttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment = 2;
	depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

	std::vector<vk::AttachmentReference> colorAttachments{ colorAttachmentRef, depthGUIAttachmentRef };

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

	std::vector<vk::AttachmentDescription> attachments = { colorAttachment, depthGUIAttachment, depthAttachment };
	vk::RenderPassCreateInfo renderPassInfo{};
	renderPassInfo.setAttachments(attachments);
	renderPassInfo.setSubpasses(subpass);
	renderPassInfo.setDependencies(dependency);
	
	renderPass = DGame->context.logical.createRenderPass(renderPassInfo);

}

void DDing::ForwardPass::InitPipeline()
{
	PipelineDesc pipelineDesc{};

	auto vertShaderCode = loadShader("Shaders/shader.vert.spv");
	vk::ShaderModuleCreateInfo vertCreateInfo{};
	vertCreateInfo.setCode(vertShaderCode);
	vk::raii::ShaderModule vertShaderModule = DGame->context.logical.createShaderModule(vertCreateInfo);
	vk::PipelineShaderStageCreateInfo vertStage{};
	vertStage.setModule(*vertShaderModule);
	vertStage.setPName("main");
	vertStage.setStage(vk::ShaderStageFlagBits::eVertex);

	auto fragShaderCode = loadShader("Shaders/shader.frag.spv");
	vk::ShaderModuleCreateInfo fragCreateInfo{};
	fragCreateInfo.setCode(fragShaderCode);
	vk::raii::ShaderModule fragShaderModule = DGame->context.logical.createShaderModule(fragCreateInfo);
	vk::PipelineShaderStageCreateInfo fragStage{};
	fragStage.setModule(*fragShaderModule);
	fragStage.setPName("main");
	fragStage.setStage(vk::ShaderStageFlagBits::eFragment);


	std::vector<vk::PipelineShaderStageCreateInfo> shaderStages = { vertStage,fragStage };
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

	vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
	colorBlendAttachment.setBlendEnable(vk::False);
	colorBlendAttachment.setSrcColorBlendFactor(vk::BlendFactor::eOne);
	colorBlendAttachment.setDstColorBlendFactor(vk::BlendFactor::eZero);
	colorBlendAttachment.setColorBlendOp(vk::BlendOp::eAdd);
	colorBlendAttachment.setSrcAlphaBlendFactor(vk::BlendFactor::eOne);
	colorBlendAttachment.setDstAlphaBlendFactor(vk::BlendFactor::eZero);
	colorBlendAttachment.setAlphaBlendOp(vk::BlendOp::eAdd);

	vk::PipelineColorBlendAttachmentState GUIBlendAttachment{};
	GUIBlendAttachment.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
	GUIBlendAttachment.setBlendEnable(vk::False);
	GUIBlendAttachment.setSrcColorBlendFactor(vk::BlendFactor::eOne);
	GUIBlendAttachment.setDstColorBlendFactor(vk::BlendFactor::eZero);
	GUIBlendAttachment.setColorBlendOp(vk::BlendOp::eAdd);
	GUIBlendAttachment.setSrcAlphaBlendFactor(vk::BlendFactor::eOne);
	GUIBlendAttachment.setDstAlphaBlendFactor(vk::BlendFactor::eZero);
	GUIBlendAttachment.setAlphaBlendOp(vk::BlendOp::eAdd);

	std::vector<vk::PipelineColorBlendAttachmentState> attachments{ colorBlendAttachment,GUIBlendAttachment };

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
	std::vector<vk::DescriptorSetLayout> setLayouts = { *sceneSetLayout,*bindlessSetLayout };
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

void DDing::ForwardPass::InitFrameData()
{

	for (int i = 0; i < FRAME_CNT; i++) {
		ForwardFrameData frameData{};
		{

			std::vector<vk::DescriptorPoolSize> poolSizes = {
				vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer,1),
				vk::DescriptorPoolSize(vk::DescriptorType::eStorageBuffer,1),
				vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler,1024),
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
			bufferInfo.setSize(sizeof(GlobalBuffer));
			bufferInfo.setUsage(vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst);

			VmaAllocationCreateInfo allocInfo{};
			allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
			allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
			allocInfo.priority = 1.0f;

			frameData.uniformBuffer = DDing::Buffer(bufferInfo, allocInfo);
		}
		{
			vk::BufferCreateInfo stagingInfo{ };
			stagingInfo.setSize(sizeof(GlobalBuffer));
			stagingInfo.setUsage(vk::BufferUsageFlagBits::eTransferSrc);

			VmaAllocationCreateInfo allocInfo{};
			allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
			allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

			frameData.stagingBuffer = DDing::Buffer(stagingInfo, allocInfo);
		}
		{
			vk::DescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = frameData.uniformBuffer.buffer;
			bufferInfo.range = sizeof(GlobalBuffer);
			bufferInfo.offset = 0;

			vk::WriteDescriptorSet descriptorWrite{};
			descriptorWrite.dstSet = frameData.descriptorSet;
			descriptorWrite.dstBinding = 0;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = vk::DescriptorType::eUniformBuffer;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;

			DGame->context.logical.updateDescriptorSets(descriptorWrite, nullptr);
		}
		frameDatas.push_back(std::move(frameData));
	}
}


void DDing::ForwardPass::Render(vk::CommandBuffer commandBuffer)
{
	SetBuffer(commandBuffer);
	auto currentFrame = DGame->render.currentFrame;
	auto currentScene = DGame->scene.currentScene;


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
	ForwardFrameData& frameData = frameDatas[currentFrame];

	std::vector<vk::DescriptorSet> descriptorSets = {
		*frameData.descriptorSet,
		*bindlessSet,
	};

	commandBuffer.beginRenderPass(renderPassbeginInfo, vk::SubpassContents::eInline);
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline);
	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipeline->GetLayout(), 0, descriptorSets, nullptr);
	//commandBuffer.draw(3, 1, 0, 0);

	for (auto& rootNode : currentScene->GetRootNodes()) {
		auto node = rootNode;
		node->Draw(commandBuffer, *pipeline->GetLayout());

	}


	commandBuffer.endRenderPass();

}

void DDing::ForwardPass::InitShadowDescriptorUpdate(RenderPass* pass)
{
	auto shadowPass = static_cast<ShadowPass*>(pass);
	auto& shadowMaps = shadowPass->outputImages;
	for (int frameCnt = 0; frameCnt < FRAME_CNT; frameCnt++) {

		auto& frameData = frameDatas[frameCnt];
		auto& shadowFrameData = shadowPass->frameDatas[frameCnt];

		std::vector<vk::WriteDescriptorSet> descriptorWrites;

		std::array<vk::DescriptorImageInfo, MAX_LIGHTS> shadowMapInfos;
		std::array<vk::DescriptorImageInfo, MAX_LIGHTS> shadowCubeMapInfos;

		for (int i = 0; i < MAX_LIGHTS; i++) {
			vk::DescriptorImageInfo shadowMapInfo{};
			shadowMapInfo.imageView = shadowMaps[frameCnt * MAX_LIGHTS * 2 + i].imageView;
			shadowMapInfo.sampler = *DGame->render.DefaultSampler;
			shadowMapInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
			shadowMapInfos[i] = shadowMapInfo;

		}
		vk::WriteDescriptorSet descriptorWrite{};
		descriptorWrite.dstSet = *frameData.descriptorSet;
		descriptorWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
		descriptorWrite.dstBinding = 1;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorCount = MAX_LIGHTS;
		descriptorWrite.pImageInfo = shadowMapInfos.data();
		descriptorWrites.push_back(descriptorWrite);

		for (int i = 0; i < MAX_LIGHTS; i++) {
			vk::DescriptorImageInfo shadowCubemapInfo{};
			shadowCubemapInfo.imageView = shadowMaps[frameCnt * MAX_LIGHTS * 2 + MAX_LIGHTS +  i].imageView;
			shadowCubemapInfo.sampler = *DGame->render.DefaultSampler;
			shadowCubemapInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
			shadowCubeMapInfos[i] = shadowCubemapInfo;

		}
		descriptorWrite.dstSet = *frameData.descriptorSet;
		descriptorWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
		descriptorWrite.dstBinding = 2;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorCount = MAX_LIGHTS;
		descriptorWrite.pImageInfo = shadowCubeMapInfos.data();
		descriptorWrites.push_back(descriptorWrite);


		vk::DescriptorBufferInfo lightMatrixInfo{};
		lightMatrixInfo.buffer = shadowFrameData.uniformBuffer.buffer;
		lightMatrixInfo.offset = 0;
		lightMatrixInfo.range = sizeof(DDing::ShadowPass::TotalShadowBuffer);

		descriptorWrite.dstSet = *frameData.descriptorSet;
		descriptorWrite.descriptorType = vk::DescriptorType::eUniformBuffer;
		descriptorWrite.dstBinding = 3;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &lightMatrixInfo;
		descriptorWrites.push_back(descriptorWrite);


		DGame->context.logical.updateDescriptorSets(descriptorWrites, {});

	}
}

void DDing::ForwardPass::DrawUI()
{
	int currentFrame = DGame->render.currentFrame;
	if (ImGui::CollapsingHeader("ForwardPass")) {
		bool pcfEnabled = (enablePCF != 0);
		if (ImGui::Checkbox("Enable PCF", &pcfEnabled)) {
			enablePCF = pcfEnabled ? 1 : 0;
		}

		ImGui::Text("Depth Image");
		ImGui::Image((ImTextureID)depthImageDescriptorSet[currentFrame], ImVec2(DGame->swapChain.extent.width * 0.3, DGame->swapChain.extent.height * 0.3));

	}
}

void DDing::ForwardPass::SetBuffer(vk::CommandBuffer commandBuffer)
{
	ForwardFrameData& frameData = frameDatas[DGame->render.currentFrame];
	auto camera = DGame->scene.currentScene->GetCamera();
	auto eyePosition = camera->GetComponent<DDing::Transform>()->GetWorldPosition();
	void* mappedData = frameData.stagingBuffer.GetMappedPtr();
	memcpy((char*)mappedData + offsetof(GlobalBuffer, view), &Camera::View, sizeof(glm::mat4));
	memcpy((char*)mappedData + offsetof(GlobalBuffer, projection), &Camera::Projection, sizeof(glm::mat4));
	memcpy((char*)mappedData + offsetof(GlobalBuffer, cameraPosition), &eyePosition, sizeof(glm::vec3));

	//Light
	int cnt = 0;
	sLight lights[10];
	for (auto& go : DGame->scene.currentScene->GetNodes()) {
		auto lightComponent = go->GetComponent<DDing::Light>();
		if (lightComponent) {
			lights[cnt].color = lightComponent->color;
			lights[cnt].intensity = lightComponent->intensity;
			lights[cnt].position = go->GetComponent<DDing::Transform>()->GetWorldPosition();
			lights[cnt].direction = go->GetComponent<DDing::Transform>()->GetLook();
			lights[cnt].type = static_cast<int>(lightComponent->type);
			lights[cnt].innerCone = glm::cos(glm::radians(lightComponent->innerCone));
			lights[cnt].outerCone = glm::cos(glm::radians(lightComponent->outerCone));
			cnt++;
		}
	}

	memcpy((char*)mappedData + offsetof(GlobalBuffer, lights), lights, sizeof(sLight) * 10);
	memcpy((char*)mappedData + offsetof(GlobalBuffer, numLights), &cnt, sizeof(int));
	memcpy((char*)mappedData + offsetof(GlobalBuffer, enablePCF), &enablePCF, sizeof(int));

	VkBufferCopy copyRegion{};
	copyRegion.dstOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = sizeof(GlobalBuffer);

	vkCmdCopyBuffer(commandBuffer, frameData.stagingBuffer.buffer, frameData.uniformBuffer.buffer, 1, &copyRegion);
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

		framebuffers.emplace_back(DGame->context.logical, framebufferInfo);
	}
}

void DDing::ForwardPass::InitDescriptors()
{

	{
		std::vector<vk::DescriptorSetLayoutBinding> bindings{ };

		vk::DescriptorSetLayoutBinding layoutBinding{};
		layoutBinding.binding = 0;
		layoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
		layoutBinding.descriptorCount = 1;
		layoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment;
		bindings.push_back(layoutBinding);

		vk::DescriptorSetLayoutBinding shadowMapBinding{};
		shadowMapBinding.binding = 1;
		shadowMapBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
		shadowMapBinding.descriptorCount = MAX_LIGHTS;
		shadowMapBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;
		bindings.push_back(shadowMapBinding);

		vk::DescriptorSetLayoutBinding shadowCubemapBinding{};
		shadowCubemapBinding.binding = 2;
		shadowCubemapBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
		shadowCubemapBinding.descriptorCount = MAX_LIGHTS;
		shadowCubemapBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;
		bindings.push_back(shadowCubemapBinding);


		vk::DescriptorSetLayoutBinding directionalMatrixBinding{};
		directionalMatrixBinding.binding = 3;
		directionalMatrixBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
		directionalMatrixBinding.descriptorCount = 1;
		directionalMatrixBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;
		bindings.push_back(directionalMatrixBinding);

		vk::DescriptorSetLayoutBinding spotMatrixBinding{};
		spotMatrixBinding.binding = 4;
		spotMatrixBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
		spotMatrixBinding.descriptorCount = 1;
		spotMatrixBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;
		bindings.push_back(spotMatrixBinding);

		vk::DescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.setBindings(bindings);
		sceneSetLayout = DGame->context.logical.createDescriptorSetLayout(layoutInfo);
	}
	{
		vk::DescriptorSetLayoutBinding materialBufferBinding{};
		materialBufferBinding.binding = 0;
		materialBufferBinding.descriptorType = vk::DescriptorType::eStorageBuffer;
		materialBufferBinding.descriptorCount = 1;
		materialBufferBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

		vk::DescriptorSetLayoutBinding textureArrayBinding{};
		textureArrayBinding.binding = 1;
		textureArrayBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
		textureArrayBinding.descriptorCount = 1024; //TODO Change
		textureArrayBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;
		textureArrayBinding.pImmutableSamplers = nullptr;


		vk::DescriptorSetLayoutBindingFlagsCreateInfo bindingFlagCreateInfo{};

		std::vector<vk::DescriptorBindingFlags> bindingFlags{
			{},
			vk::DescriptorBindingFlagBits::ePartiallyBound | vk::DescriptorBindingFlagBits::eUpdateAfterBind,
		};
		bindingFlagCreateInfo.setBindingFlags(bindingFlags);

		std::vector<vk::DescriptorSetLayoutBinding> bindings{
			materialBufferBinding,textureArrayBinding,
		};
		vk::DescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.bindingCount = 2;
		layoutInfo.setBindings(bindings);
		layoutInfo.flags = vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPoolEXT;
		layoutInfo.pNext = &bindingFlagCreateInfo;

		bindlessSetLayout = DGame->context.logical.createDescriptorSetLayout(layoutInfo);

	}
	{
		std::vector<vk::DescriptorPoolSize> poolSizes = {
			vk::DescriptorPoolSize(vk::DescriptorType::eStorageBuffer,1),
			vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler,1024),
		};

		vk::DescriptorPoolCreateInfo poolInfo{};
		poolInfo.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet | vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind);
		poolInfo.setPoolSizes(poolSizes);
		poolInfo.setMaxSets(2);

		bindlessPool = DGame->context.logical.createDescriptorPool(poolInfo);
	}
	{

		std::vector<uint32_t> variableDescriptorCounts = { 1024 };
		vk::DescriptorSetVariableDescriptorCountAllocateInfo variableDescriptorCountAllocInfo{};
		variableDescriptorCountAllocInfo.setDescriptorCounts(variableDescriptorCounts);

		vk::DescriptorSetAllocateInfo allocInfo{};
		allocInfo.setDescriptorPool(*bindlessPool);
		allocInfo.setDescriptorSetCount(1);
		allocInfo.setSetLayouts(*bindlessSetLayout);
		allocInfo.setPNext(&variableDescriptorCountAllocInfo);

		bindlessSet = std::move(DGame->context.logical.allocateDescriptorSets(allocInfo).front());
	}

	DGame->resource.gltfs[0].updateDescriptorSet(*bindlessSet);

}

void DDing::ForwardPass::initDepthImageGUI()
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
		imageInfo.setUsage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled);
		imageInfo.setInitialLayout(vk::ImageLayout::eUndefined);
		imageInfo.setSharingMode(vk::SharingMode::eExclusive);

		vk::ImageViewCreateInfo imageViewInfo{};
		imageViewInfo.setFormat(imageInfo.format);
		imageViewInfo.setViewType(vk::ImageViewType::e2D);
		imageViewInfo.setSubresourceRange({ vk::ImageAspectFlagBits::eColor, 0, imageInfo.mipLevels, 0, 1 });

		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
		allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
		allocInfo.priority = 1.0f;


		depthImageGUI.push_back(DDing::Image(imageInfo, allocInfo, imageViewInfo));
		DGame->context.immediate_submit([&](vk::CommandBuffer commandBuffer) {
			depthImageGUI[i].setImageLayout(commandBuffer, vk::ImageLayout::eShaderReadOnlyOptimal);
			});


		depthImageDescriptorSet.push_back(ImGui_ImplVulkan_AddTexture(*DGame->render.DefaultSampler, depthImageGUI[i].imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));

	}





}