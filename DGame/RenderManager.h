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

	//For Global Descriptor
	vk::raii::DescriptorPool descriptorPool = nullptr;
	vk::raii::DescriptorSet descriptorSet = nullptr;
	DDing::Buffer globalUniformBuffer;
	DDing::Buffer globalStagingBuffer;
	GlobalBuffer buffer;
};
class RenderManager
{
public:
	RenderManager() {};
	void Init();
	void DrawFrame(DDing::Scene* scene, DDing::PassType passType);

	uint32_t currentFrame = 0;
	//TODO
	DDing::Pipeline* currentPipeline;
	vk::RenderPass currentRenderPass;
	std::vector<FrameData> frameDatas = {};
	vk::raii::DescriptorSetLayout globalSetLayout = nullptr;
	vk::raii::DescriptorSetLayout bindLessLayout = nullptr;
private:
	void initRenderPasses();
	void initPipelines();
	void initPasses();
	void initGlobalDescriptorSetLayout();
	void initBindLessDescriptorSetLayout();
	void initFrameDatas();

	void submitCommandBuffer(vk::CommandBuffer commandBuffer);
	void presentCommandBuffer(vk::CommandBuffer commandBuffer,uint32_t imageIndex);

	void copyResultToSwapChain(vk::CommandBuffer commandBuffer,uint32_t imageIndex);
	void copyGlobalBuffer(vk::CommandBuffer commandBuffer);

	std::unordered_map<DDing::RenderPassType, vk::raii::RenderPass> renderPasses;
	std::unordered_map<DDing::PipelineType, std::unique_ptr<DDing::Pipeline>> pipelines;
	std::unordered_map<DDing::PassType, std::unique_ptr<DDing::RenderPass>> passes;



};

