#pragma once
#define FRAME_CNT 2
#include "Pipeline.h"
#include "RenderPass.h"
namespace DDing {
	enum class PipelineType {
		Default,
	};
	enum class RenderPassType {
		Default,
	};
	enum class PassType {
		Default,
	};
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
	void DrawFrame(DDing::Scene& scene, DDing::PassType passType);
private:
	void initRenderPasses();
	void initPipelines();
	void initPasses();
	void initDescriptors();

	void submitCommandBuffer();
	void presentCommandBuffer();

	std::unordered_map<DDing::RenderPassType, vk::raii::RenderPass> renderPasses;
	std::unordered_map<DDing::PipelineType, std::unique_ptr<DDing::Pipeline>> pipelines;
	std::unordered_map<DDing::PassType, std::unique_ptr<DDing::RenderPass>> passes;
	std::array<FrameData, FRAME_CNT> frameDatas = {};

	DDing::Context* context;
	DDing::SwapChain* swapChain;
	
	uint8_t currentFrame = 0;
	//TODO Objects
};

