#include "pch.h"
#include "RenderManager.h"
#include "Pipeline.h"
#include "SwapChain.h"
#include "RenderPass.h"
#include "Context.h"
void RenderManager::Init()
{
    initFrameDatas();
    initRenderPasses();
    initPipelines();
    initPasses();
}
void RenderManager::DrawFrame(DDing::Scene* scene, DDing::PassType passType)
{
    FrameData& frameData = frameDatas[currentFrame];

    auto resultForFence = DGame->context.logical.waitForFences({ *frameData.waitFrame }, vk::True, UINT64_MAX);
    
    auto acquireImage = DGame->swapChain.Get().acquireNextImage(UINT64_MAX, *frameData.imageAvaiable, VK_NULL_HANDLE);
    uint32_t imageIndex = acquireImage.second;
    vk::Result result = acquireImage.first;
    //TODO handle recreateSwapchain
    DGame->context.logical.resetFences({ *frameData.waitFrame });

    frameData.commandBuffer.reset({});
    vk::CommandBufferBeginInfo beginInfo{};
    frameData.commandBuffer.begin(beginInfo);
    copyGlobalBuffer(*frameData.commandBuffer);

    auto passIt = passes.find(passType);
    if (passIt == passes.end())
        throw std::runtime_error("RenderPass type not found");

    passes[passType]->Render(frameData.commandBuffer, scene);
    
    //First copy to Swapchain, and draw gui on swapChain
    copyResultToSwapChain(*frameData.commandBuffer, imageIndex);
    DGame->input.DrawImGui(*frameData.commandBuffer, imageIndex);
    
    frameData.commandBuffer.end();
    
    submitCommandBuffer(*frameData.commandBuffer);
    presentCommandBuffer(*frameData.commandBuffer, imageIndex);


    currentFrame = (currentFrame + 1) % FRAME_CNT;
}
void RenderManager::initRenderPasses()
{
    //Default
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

        vk::AttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

        vk::SubpassDescription subpass{};
        subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        vk::SubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
        dependency.srcAccessMask = {};
        dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
        dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

        std::vector<vk::AttachmentDescription> attachments = { colorAttachment,depthAttachment };
        vk::RenderPassCreateInfo renderPassInfo{};
        renderPassInfo.setAttachments(attachments);
        renderPassInfo.setSubpasses(subpass);
        renderPassInfo.setDependencies(dependency);
		vk::raii::RenderPass renderPass = DGame->context.logical.createRenderPass(renderPassInfo);

        renderPasses.insert({ DDing::RenderPassType::Default,std::move(renderPass) });
	}
    //Deferred
	{

	}
}

void RenderManager::initPipelines()
{
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
       

        std::vector<vk::PipelineShaderStageCreateInfo> shaderStages = { vertStage,fragStage};
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
        
        vk::PipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.setLogicOpEnable(vk::False);
        colorBlending.setLogicOp(vk::LogicOp::eCopy);
        colorBlending.setAttachments(colorBlendAttachment);
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
        vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.setSetLayouts({*globalSetLayout});
        vk::PushConstantRange pushConstantRange{};
        pushConstantRange.setOffset(0);
        pushConstantRange.setSize(sizeof(DDing::ForwardPass::PushConstant));
        pushConstantRange.setStageFlags(vk::ShaderStageFlagBits::eVertex);
        pipelineLayoutInfo.setPushConstantRanges({pushConstantRange});
        pipelineDesc.layout = pipelineLayoutInfo;

        //TODO
        pipelineDesc.renderPass = *renderPasses.at(DDing::RenderPassType::Default);


        auto pipeline = std::make_unique<DDing::GraphicsPipeline>(DGame->context,pipelineDesc);
        pipelines.insert({ DDing::PipelineType::Default,std::move(pipeline)});
    }
}

void RenderManager::initPasses()
{
    //ForwardPass
    {
        auto forwardPass = std::make_unique<DDing::ForwardPass>(*pipelines.at(DDing::PipelineType::Default), *renderPasses.at(DDing::RenderPassType::Default));

        //TODO currentPipeline change
        currentPipeline = pipelines.at(DDing::PipelineType::Default).get();
        currentRenderPass = *renderPasses.at(DDing::RenderPassType::Default);

        passes.insert({ DDing::PassType::Default,std::move(forwardPass) });

    }
}

void RenderManager::initGlobalDescriptorSetLayout()
{
    vk::DescriptorSetLayoutBinding layoutBinding{};
    layoutBinding.binding = 0;
    layoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
    layoutBinding.descriptorCount = 1;
    layoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment;

    vk::DescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.setBindings(layoutBinding);
    globalSetLayout = DGame->context.logical.createDescriptorSetLayout(layoutInfo);
}

void RenderManager::initFrameDatas()
{
    for (int i = 0; i < FRAME_CNT; i++) {
        FrameData frameData{};
        {
            vk::SemaphoreCreateInfo semaphoreInfo{};
            frameData.imageAvaiable = DGame->context.logical.createSemaphore(semaphoreInfo);
            frameData.renderFinish = DGame->context.logical.createSemaphore(semaphoreInfo);
        }
        {
            vk::FenceCreateInfo fenceInfo{};
            fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;
            frameData.waitFrame = DGame->context.logical.createFence(fenceInfo);
        }
        {
            vk::CommandPoolCreateInfo commandPoolInfo{};
            QueueFamilyIndices indices = DGame->context.findQueueFamilies(*DGame->context.physical, *DGame->context.surface);
            commandPoolInfo.setQueueFamilyIndex(indices.graphicsFamily.value());
            commandPoolInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
            frameData.commandPool = DGame->context.logical.createCommandPool(commandPoolInfo);
        }
        {
            vk::CommandBufferAllocateInfo allocInfo{};
            allocInfo.setLevel(vk::CommandBufferLevel::ePrimary);
            allocInfo.setCommandBufferCount(1);
            allocInfo.setCommandPool(*frameData.commandPool);

            frameData.commandBuffer = std::move(DGame->context.logical.allocateCommandBuffers(allocInfo).front());
        }
        //Descriptor Sets, buffers For Global Variables
        {
            initGlobalDescriptorSetLayout();
            {
                vk::DescriptorPoolSize poolSize{};
                poolSize.setType(vk::DescriptorType::eUniformBuffer);
                poolSize.setDescriptorCount(FRAME_CNT);

                vk::DescriptorPoolCreateInfo poolInfo{};
                poolInfo.setMaxSets(FRAME_CNT);
                poolInfo.setPoolSizes(poolSize);
                poolInfo.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);

                frameData.descriptorPool = DGame->context.logical.createDescriptorPool(poolInfo);
            }
            {
                vk::DescriptorSetAllocateInfo allocInfo{};
                allocInfo.setDescriptorPool(*frameData.descriptorPool);
                allocInfo.setDescriptorSetCount(FRAME_CNT);
                allocInfo.setSetLayouts(*globalSetLayout);

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

                frameData.globalUniformBuffer = DDing::Buffer(bufferInfo, allocInfo);
            }
            {
                vk::BufferCreateInfo stagingInfo{ };
                stagingInfo.setSize(sizeof(GlobalBuffer));
                stagingInfo.setUsage(vk::BufferUsageFlagBits::eTransferSrc);

                VmaAllocationCreateInfo allocInfo{};
                allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
                allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

                frameData.globalStagingBuffer = DDing::Buffer(stagingInfo, allocInfo);
            }
            {
                vk::DescriptorBufferInfo bufferInfo{};
                bufferInfo.buffer = frameData.globalUniformBuffer.buffer;
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
        }
        frameDatas.push_back(std::move(frameData));
    }
}

void RenderManager::submitCommandBuffer(vk::CommandBuffer commandBuffer)
{
    vk::SubmitInfo submitInfo{};
    FrameData& frameData = frameDatas[currentFrame];

    vk::PipelineStageFlags waitStages[1] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
    submitInfo.setCommandBuffers(commandBuffer);
    submitInfo.setSignalSemaphores(*frameData.renderFinish);
    submitInfo.setWaitSemaphores(*frameData.imageAvaiable);
    submitInfo.setWaitDstStageMask(waitStages);

    DGame->context.GetQueue(DDing::Context::QueueType::GRAPHICS).submit(submitInfo, *frameData.waitFrame);
};

void RenderManager::presentCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t imageIndex)
{
    vk::PresentInfoKHR presentInfo{};
    FrameData& frameData = frameDatas[currentFrame];

    presentInfo.setSwapchains(*(DGame->swapChain.Get()));
    presentInfo.setWaitSemaphores(*frameData.renderFinish);
    presentInfo.setImageIndices(imageIndex);
    presentInfo.setPResults(nullptr);

    auto result = DGame->context.GetQueue(DDing::Context::QueueType::PRESENT).presentKHR(presentInfo);
}

void RenderManager::copyResultToSwapChain(vk::CommandBuffer commandBuffer, uint32_t imageIndex)
{
    DDing::Image::setImageLayout(commandBuffer, DGame->swapChain.images[imageIndex], vk::ImageLayout::ePresentSrcKHR, vk::ImageLayout::eTransferDstOptimal);
    
    //TODO current Pass change not default
    auto& renderedImage = passes.at(DDing::PassType::Default)->GetOutputImage();
    renderedImage.setImageLayout(commandBuffer, vk::ImageLayout::eTransferSrcOptimal);
    
    
    vk::ImageSubresourceLayers subresourceLayers{};
    subresourceLayers.aspectMask = vk::ImageAspectFlagBits::eColor;
    subresourceLayers.baseArrayLayer = 0;
    subresourceLayers.layerCount = 1;
    subresourceLayers.mipLevel = 0;
    vk::ImageCopy copyRegion{};
    copyRegion.srcSubresource = subresourceLayers;
    copyRegion.srcOffset = vk::Offset3D{ 0, 0, 0 };
    copyRegion.dstSubresource = subresourceLayers;
    copyRegion.dstOffset = vk::Offset3D{ 0, 0, 0 };
    copyRegion.extent = vk::Extent3D{ DGame->swapChain.extent.width, DGame->swapChain.extent.height, 1 };
    commandBuffer.copyImage(renderedImage.image,
        vk::ImageLayout::eTransferSrcOptimal,
        DGame->swapChain.images[imageIndex],
        vk::ImageLayout::eTransferDstOptimal,
        copyRegion);

    //For
    DDing::Image::setImageLayout(commandBuffer, DGame->swapChain.images[imageIndex], vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eColorAttachmentOptimal);
    renderedImage.setImageLayout(commandBuffer, vk::ImageLayout::eColorAttachmentOptimal);


}

void RenderManager::copyGlobalBuffer(vk::CommandBuffer commandBuffer)
{
    FrameData& frameData = frameDatas[currentFrame];
    VkBufferCopy copyRegion{};
    copyRegion.dstOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = sizeof(GlobalBuffer);

    vkCmdCopyBuffer(commandBuffer, frameData.globalStagingBuffer.buffer, frameData.globalUniformBuffer.buffer, 1, &copyRegion);
}
