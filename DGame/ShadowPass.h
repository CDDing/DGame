#pragma once
#include "RenderPass.h"
namespace DDing
{
	class ShadowPass : public RenderPass
	{
		struct alignas(64) ShadowBuffer {
			glm::mat4 view;
			glm::mat4 projection;
			glm::vec3 lightPosition;
		};
		struct alignas(16) TotalShadowBuffer {
			ShadowBuffer directional[MAX_LIGHTS];
			ShadowBuffer point[MAX_LIGHTS][6];
			ShadowBuffer spot[MAX_LIGHTS];

		};
		struct ShadowFrameData {
			vk::raii::DescriptorPool descriptorPool = nullptr;
			vk::raii::DescriptorSet descriptorSet = nullptr;

			DDing::Buffer uniformBuffer;
			DDing::Buffer stagingBuffer;

			std::vector<std::vector<vk::raii::ImageView>> pointLightShadowMapViews;
		};


	public:
		static vk::Format DepthFormat;
		static vk::Format SampleFormat;
		ShadowPass();
		~ShadowPass();
		void InitRenderPass() override;
		void InitPipeline() override;
		void InitFrameData() override;
		void InitDescriptors() override;
		void Render(vk::CommandBuffer commandBuffer) override;
		void DrawUI() override;

		void createOutputImages() override;
	protected:
		void SetBuffer(vk::CommandBuffer commandBuffer);
		uint32_t GetLength() { return std::min(DGame->swapChain.extent.width, DGame->swapChain.extent.height); }
		void createDepthImage();
		void createPointLightShadowMapViews();
		void createFramebuffers();
		DDing::Image depthImage;


		std::vector<ShadowFrameData> frameDatas;

		vk::raii::DescriptorSetLayout sceneSetLayout = nullptr;

		std::vector<vk::raii::Framebuffer> pointLightFramebuffers;
		std::vector<vk::raii::Framebuffer> directionalLightFramebuffers;
		std::vector<vk::raii::Framebuffer> spotLightFramebuffers;

		uint32_t directionalLightCnt = 0, pointLightCnt = 0, spotLightCnt = 0;


	};
};

