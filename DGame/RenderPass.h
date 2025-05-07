#pragma once
namespace DDing {
	class Scene;
	class RenderPass
	{
	public:
		RenderPass() {};
		virtual ~RenderPass() = default;
		virtual void Render(vk::CommandBuffer commandBuffer, DDing::Scene* scene) = 0;
		virtual void DrawUI() {};
		
		DDing::Image& GetOutputImage();

	protected:
		RenderPass(Pipeline& pipeline, vk::RenderPass renderPass) : pipeline(&pipeline), renderPass(renderPass) {};
		Pipeline* pipeline = nullptr;
		vk::RenderPass renderPass = nullptr;
		virtual void createOutputImages();
		std::vector<DDing::Image> outputImages;
	};
	class ForwardPass : public RenderPass
	{
	public:
		struct PushConstant {
			glm::mat4 transformMatrix;
			vk::DeviceAddress deviceAddress;
			int materialIndex;
		};
		static vk::Format DepthFormat;
		static vk::Format ColorFormat;
		ForwardPass(Pipeline& pipeline, vk::RenderPass renderPass);
		~ForwardPass() override;
		virtual void Render(vk::CommandBuffer commandBuffer, DDing::Scene* scene);
		void DrawUI() override;
	protected:
		void createDepthImage();
		void createFramebuffers();
		
		void initDepthImageGUI();
		DDing::Image depthImage;

		vk::raii::Sampler depthImageSampler = nullptr;
		std::vector<VkDescriptorSet> depthImageDescriptorSet;
		std::vector<DDing::Image> depthImageGUI;
		
		std::vector<vk::raii::Framebuffer> framebuffers;

	};
	class DeferredPass :public RenderPass
	{
	public:
		DeferredPass(Pipeline& pipeline, vk::RenderPass renderPass);
		virtual void Render(vk::CommandBuffer commandBuffer, DDing::Scene* scene);
	};
	class ShadowPass : public RenderPass
	{
	public:
		ShadowPass(Pipeline& pipeline, vk::RenderPass renderPass);
		virtual void Render(vk::CommandBuffer commandBuffer, DDing::Scene* scene);

		void createOutputImages() override;
	protected:
		void createFramebuffers();

		std::vector<vk::raii::Framebuffer> framebuffers;

	};
}

