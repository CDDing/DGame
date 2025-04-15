#include "pch.h"
#include "RenderManager.h"
#include "Pipeline.h"
RenderManager::RenderManager(DDing::Context& context, DDing::SwapChain& swapChain) : 
	context(&context), swapChain(&swapChain)
{
	initRenderPasses();
}
void RenderManager::DrawFrame(DDing::Scene& scene, std::string passType)
{
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
        depthAttachment.format = swapChain->depthFormat;
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

        renderPasses.insert({ std::string("default"),std::move(renderPass) });
	}
    //Deferred
	{

	}
}

void RenderManager::initPipelines()
{
    {
        auto shader = loadShader("Directory");

    }
}

void RenderManager::initDescriptors()
{
}
