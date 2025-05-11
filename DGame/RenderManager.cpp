#include "pch.h"
#include "RenderManager.h"
#include "Pipeline.h"
#include "SwapChain.h"
#include "RenderPass.h"
#include "ForwardPass.h"
#include "DeferredPass.h"
#include "ShadowPass.h"
#include "Context.h"
void RenderManager::Init()
{
    InitGUISampler();
    initFrameDatas();
    initPasses();
}
void RenderManager::DrawFrame(DDing::Scene* scene)
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

    //Shadow
    {


        passes[DDing::PassType::eShadow]->Render(frameData.commandBuffer);
    }

    //Forward
    {

        
        passes[DDing::PassType::eForward]->Render(frameData.commandBuffer);

        //First copy to Swapchain, and draw gui on swapChain
        copyResultToSwapChain(*frameData.commandBuffer, imageIndex);
        DGame->input.DrawImGui(*frameData.commandBuffer, imageIndex);

        frameData.commandBuffer.end();

    }
    //PostProcessing
    {

    }
    submitCommandBuffer(*frameData.commandBuffer);
    presentCommandBuffer(*frameData.commandBuffer, imageIndex);


    currentFrame = (currentFrame + 1) % FRAME_CNT;
}
void RenderManager::DrawUI()
{
    for (auto& element : passes) {
        auto& pass = element.second;
        pass->DrawUI();
    }
}

void RenderManager::InitGUISampler()
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
    DefaultSampler = vk::raii::Sampler(DGame->context.logical, samplerInfo);
}

void RenderManager::initPasses()
{
    //ShadowPass
    {
        auto shadowPass = std::make_unique<DDing::ShadowPass>();

        passes.insert({ DDing::PassType::eShadow,std::move(shadowPass) });
    }
    //ForwardPass
    {
        auto forwardPass = std::make_unique<DDing::ForwardPass>();
        forwardPass->InitShadowDescriptorUpdate(passes[DDing::PassType::eShadow]->outputImages);

        passes.insert({ DDing::PassType::eForward,std::move(forwardPass) });

    }
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
    auto& renderedImage = passes.at(DDing::PassType::eForward)->outputImages[currentFrame];
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
