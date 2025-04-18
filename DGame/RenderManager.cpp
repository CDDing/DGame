#include "pch.h"
#include "RenderManager.h"
#include "Pipeline.h"
#include "SwapChain.h"
#include "RenderPass.h"
#include "Context.h"
RenderManager::RenderManager(DDing::Context& context, DDing::SwapChain& swapChain) : 
	context(&context), swapChain(&swapChain)
{
	initRenderPasses();
    initPipelines();
    initRenderPasses();
}
void RenderManager::DrawFrame(DDing::Scene& scene, DDing::PassType passType)
{
    FrameData& frameData = frameDatas[currentFrame];

    auto resultForFence = context->logical.waitForFences({ *frameData.waitFrame }, vk::True, UINT64_MAX);
    context->logical.resetFences({ *frameData.waitFrame });

    auto acquireImage = swapChain->Get().acquireNextImage(UINT64_MAX, *frameData.imageAvaiable, VK_NULL_HANDLE);
    uint32_t imageIndex = acquireImage.second;
    vk::Result result = acquireImage.first;
    //TODO handle recreateSwapchain

    frameData.commandBuffer.reset({});
    vk::CommandBufferBeginInfo beginInfo{};
    frameData.commandBuffer.begin(beginInfo);

    auto passIt = passes.find(passType);
    if (passIt == passes.end())
        throw std::runtime_error("RenderPass type not found");

    passes[passType]->Render(frameData.commandBuffer, scene);
    
    frameData.commandBuffer.end();
    
    submitCommandBuffer();
    presentCommandBuffer();

    currentFrame = (currentFrame + 1) % FRAME_CNT;
}
void RenderManager::initRenderPasses()
{
    //Default
	{
        vk::AttachmentDescription colorAttachment{};
        colorAttachment.format = swapChain->imageFormat;
        colorAttachment.samples = vk::SampleCountFlagBits::e1;
        colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
        colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
        colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
        colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

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
		vk::raii::RenderPass renderPass = context->logical.createRenderPass(renderPassInfo);

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
        vk::raii::ShaderModule vertShaderModule = context->logical.createShaderModule(vertCreateInfo);
        vk::PipelineShaderStageCreateInfo vertStage{};
        vertStage.setModule(*vertShaderModule);
        vertStage.setPName("main");
        vertStage.setStage(vk::ShaderStageFlagBits::eVertex);

        auto fragShaderCode = loadShader("Shaders/shader.frag.spv");
        vk::ShaderModuleCreateInfo fragCreateInfo{};
        fragCreateInfo.setCode(fragShaderCode);
        vk::raii::ShaderModule fragShaderModule = context->logical.createShaderModule(fragCreateInfo);
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
        viewport.setWidth(swapChain->extent.width);
        viewport.setHeight(swapChain->extent.height);
        viewport.setX(0.0f);
        viewport.setY(0.0f);
        viewport.setMinDepth(0.0f);
        viewport.setMaxDepth(1.0f);

        vk::Rect2D scissor{};
        scissor.setExtent(swapChain->extent);

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
        pipelineLayoutInfo.setSetLayouts({});
        pipelineLayoutInfo.setPushConstantRanges({});
        pipelineDesc.layout = pipelineLayoutInfo;

        //TODO
        pipelineDesc.renderPass = *renderPasses.at(DDing::RenderPassType::Default);


        auto pipeline = std::make_unique<DDing::GraphicsPipeline>(*context,pipelineDesc);
        pipelines.insert({ DDing::PipelineType::Default,std::move(pipeline)});
    }
}

void RenderManager::initPasses()
{
    //ForwardPass
    {
        //auto forwardPass = std::make_unique<DDing::ForwardPass>(pipelines.at(DDing::PipelineType::Default), *renderPasses.at(DDing::RenderPassType::Default));
    }
}

void RenderManager::initDescriptors()
{
}

void RenderManager::submitCommandBuffer()
{
}

void RenderManager::presentCommandBuffer()
{
}
