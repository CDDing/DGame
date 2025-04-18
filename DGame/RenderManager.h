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

	vk::raii::CommandPool commandPool = nullptr;
	vk::raii::CommandBuffer commandBuffer = nullptr;
};
class RenderManager
{
public:
	RenderManager() {};
	void Init();
	void DrawFrame(DDing::Scene& scene, DDing::PassType passType);

	uint32_t currentFrame = 0;
private:
	void initRenderPasses();
	void initPipelines();
	void initPasses();
	void initDescriptors();
	void initFrameDatas();

	void submitCommandBuffer(vk::CommandBuffer commandBuffer);
	void presentCommandBuffer(vk::CommandBuffer commandBuffer,uint32_t imageIndex);

	void copyResultToSwapChain(vk::CommandBuffer commandBuffer,uint32_t imageIndex);
	std::unordered_map<DDing::RenderPassType, vk::raii::RenderPass> renderPasses;
	std::unordered_map<DDing::PipelineType, std::unique_ptr<DDing::Pipeline>> pipelines;
	std::unordered_map<DDing::PassType, std::unique_ptr<DDing::RenderPass>> passes;
	std::vector<FrameData> frameDatas = {};

	
	//TODO Objects
};

