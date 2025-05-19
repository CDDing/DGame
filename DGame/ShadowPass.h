#pragma once
#include "RenderPass.h"
namespace DDing
{
	class ShadowPass : public RenderPass
	{


	public:
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
			std::vector<vk::raii::Framebuffer> shadowMapFramebuffers;
			std::vector<vk::raii::Framebuffer> shadowCubeMapFramebuffers;
		};
		static vk::Format DepthFormat;
		ShadowPass();
		~ShadowPass();
		void InitRenderPass() override;
		void InitPipeline() override;
		void InitFrameData() override;
		void InitDescriptors() override;
		void Render(vk::CommandBuffer commandBuffer) override;
		void DrawUI() override;

		void createOutputImages() override;
		std::vector<ShadowFrameData> frameDatas;
	protected:
		void SetBuffer(vk::CommandBuffer commandBuffer);
		uint32_t GetLength() { return 2048;/*std::min(DGame->swapChain.extent.width, DGame->swapChain.extent.height);*/ }
		void createPointLightShadowMapViews();
		void createFramebuffers();



		vk::raii::DescriptorSetLayout sceneSetLayout = nullptr;



	};
};

