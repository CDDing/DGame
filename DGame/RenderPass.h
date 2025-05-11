#pragma once
namespace DDing {
	class Scene;
	class RenderPass
	{
	public:
		RenderPass() {};
		virtual ~RenderPass() = default;
		virtual void InitRenderPass() = 0;
		virtual void InitPipeline() = 0;
		virtual void InitFrameData() = 0;
		virtual void InitDescriptors() = 0;
		virtual void Render(vk::CommandBuffer commandBuffer) = 0;
		virtual void DrawUI() {};

		std::vector<DDing::Image> outputImages;

	protected:
		std::unique_ptr<Pipeline> pipeline = nullptr;
		vk::raii::RenderPass renderPass = nullptr;
		virtual void createOutputImages();
	};
	
}

