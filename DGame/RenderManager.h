#pragma once
#define FRAME_CNT 2
#include "Pipeline.h"
#include "RenderPass.h"
namespace DDing {
	enum class PassType {
		eForward,
		eShadow,
	};
	class Camera;
	class Scene;
	class SwapChain;
	class Pipeline;
}
struct FrameData {
	//For Synchronization
	vk::raii::Semaphore renderFinish = nullptr;
	vk::raii::Semaphore imageAvaiable = nullptr;
	vk::raii::Fence waitFrame = nullptr;

	//For Command
	vk::raii::CommandPool commandPool = nullptr;
	vk::raii::CommandBuffer commandBuffer = nullptr;

};
class RenderManager
{
public:
	RenderManager() {};
	void Init();
	void DrawFrame(DDing::Scene* scene);
	void DrawUI();

	uint32_t currentFrame = 0;
	vk::raii::Sampler DefaultSampler = nullptr;

	std::vector<FrameData> frameDatas = {};
private:
	void InitGUISampler();
	void initPasses();
	void initFrameDatas();

	void submitCommandBuffer(vk::CommandBuffer commandBuffer);
	void presentCommandBuffer(vk::CommandBuffer commandBuffer,uint32_t imageIndex);

	void copyResultToSwapChain(vk::CommandBuffer commandBuffer,uint32_t imageIndex);

	std::unordered_map<DDing::PassType, std::unique_ptr<DDing::RenderPass>> passes;



};

