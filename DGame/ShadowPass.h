#pragma once
#include "RenderPass.h"
namespace DDing
{
	class ShadowPass : public RenderPass
	{
	public:
		static vk::Format DepthFormat;
		ShadowPass();
		~ShadowPass();
		void InitRenderPass() override;
		void InitPipeline() override;
		void InitFrameData() override;
		void Render(vk::CommandBuffer commandBuffer) override;
		void DrawUI() override;

		void createOutputImages() override;
	protected:
		void createPointLightShadowMapViews();
		void createDescriptors();
		void createFramebuffers();

		std::vector<vk::raii::ImageView> pointLightShadowMapViews;
		std::vector<vk::raii::Framebuffer> pointLightFramebuffers;
		std::vector<vk::raii::Framebuffer> directionalLightFramebuffers;
		std::vector<vk::raii::Framebuffer> spotLightFramebuffers;

		uint32_t GetPointLightIndex(int frameCnt, int lightCnt, int faceIndex);

		//Temp
		vk::raii::DescriptorPool descriptorPool = nullptr;
		std::vector<vk::raii::DescriptorSet> descriptorSets;

		std::vector<DDing::Buffer> uniformBuffers;
		std::vector<DDing::Buffer> stagingBuffers;
	};
};

