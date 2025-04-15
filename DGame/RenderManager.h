#pragma once
namespace DDing {
	class Pipeline;
	class RenderPass;
	class Scene;
	class SwapChain;
}
struct FrameData {
	vk::raii::Semaphore renderFinish = nullptr;
	vk::raii::Semaphore imageAvaiable = nullptr;
	vk::raii::Fence waitFrame = nullptr;

	vk::raii::CommandBuffer commandBuffer = nullptr;
};
class RenderManager
{
public:
	RenderManager(DDing::Context& context, DDing::SwapChain& swapChain);
	void DrawFrame(DDing::Scene& scene, std::string passType);
private:
	void initRenderPasses();
	void initPipelines();
	void initDescriptors();

	std::string currentRenderPass;
	std::string currentPipeline;
	std::unordered_map<std::string, vk::raii::RenderPass> renderPasses;
	std::unordered_map<std::string, DDing::Pipeline> pipelines;
	std::unordered_map<std::string, DDing::RenderPass> passes;
	std::array<FrameData, FRAME_CNT> frameData = {};

	DDing::Context* context;
	DDing::SwapChain* swapChain;
	
	//TODO Objects
};

