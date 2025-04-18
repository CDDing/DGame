#pragma once
namespace DDing {
	class Scene;
	class RenderPass
	{
	public:
		RenderPass() {};
		virtual ~RenderPass() = default;
		virtual void Render(vk::CommandBuffer commandBuffer, DDing::Scene& scene) = 0;
		
	protected:
		RenderPass(Pipeline& pipeline, vk::RenderPass renderPass) : pipeline(&pipeline), renderPass(renderPass) {};
		Pipeline* pipeline = nullptr;
		vk::RenderPass renderPass = nullptr;
	};
	class ForwardPass : public RenderPass
	{
	public:
		static vk::Format DepthFormat;
		static vk::Format ColorFormat;
		ForwardPass(Pipeline& pipeline, vk::RenderPass renderPass);
		virtual void Render(vk::CommandBuffer commandBuffer, DDing::Scene& scene);
	protected:
		void createDepthImage();
		void createOutputImages();
		void createFramebuffers();
		DDing::Image depthImage;
		
		std::vector<DDing::Image> outputImages;
		std::vector<vk::raii::Framebuffer> framebuffers;

	};
	class DeferredPass :public RenderPass
	{
	public:
		DeferredPass(Pipeline& pipeline, vk::RenderPass renderPass);
		virtual void Render(vk::CommandBuffer commandBuffer, DDing::Scene& scene);
	};
}

