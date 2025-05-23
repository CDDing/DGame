#pragma once
#include "RenderPass.h"
#include "ShadowPass.h"
namespace DDing {
	class ForwardPass : public RenderPass
	{
		struct ForwardFrameData {
			vk::raii::DescriptorPool descriptorPool = nullptr;
			vk::raii::DescriptorSet descriptorSet = nullptr;

			DDing::Buffer uniformBuffer;
			DDing::Buffer stagingBuffer;
		};
	public:
		static vk::Format DepthFormat;
		static vk::Format ColorFormat;
		ForwardPass();
		~ForwardPass() override;
		void InitRenderPass() override;
		void InitPipeline() override;
		void InitFrameData() override;
		void InitDescriptors() override;
		void Render(vk::CommandBuffer commandBuffer) override;
		void InitShadowDescriptorUpdate(RenderPass* pass);
		void DrawUI() override;
	protected:
		void SetBuffer(vk::CommandBuffer commandBuffer);
		void createDepthImage();
		void createFramebuffers();

		void initDepthImageGUI();

		vk::raii::DescriptorSetLayout sceneSetLayout = nullptr;

		vk::raii::DescriptorPool bindlessPool = nullptr;
		vk::raii::DescriptorSet bindlessSet = nullptr;
		vk::raii::DescriptorSetLayout bindlessSetLayout = nullptr;

		std::vector<vk::raii::Framebuffer> framebuffers;

		//FrameData
		std::vector<ForwardFrameData> frameDatas;

		//Depth Image
		DDing::Image depthImage;

		//For GUI
		std::vector<VkDescriptorSet> depthImageDescriptorSet;
		std::vector<DDing::Image> depthImageGUI;
		int enablePCF = false;

		

	};
}

