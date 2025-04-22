#pragma once
namespace DDing {
	struct Vertex {
		glm::vec3 position;
		float pad;
		glm::vec3 normal;
		float pad2;
		glm::vec2 texcoord;
		glm::vec2 pad3;
	};
	class Primitive {

	public:
		Primitive();
		Primitive(std::vector<Vertex> vertices, std::vector<uint32_t> indices, DDing::Material* material);
		void Draw(vk::CommandBuffer commandBuffer, const glm::mat4& transform, const vk::PipelineLayout pipelineLayout);

		vk::DeviceAddress vertexBufferAddress = 0;
	private:
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		DDing::Buffer vertexBuffer;
		DDing::Buffer indexBuffer;

		DDing::Material* material;

	};
	class Mesh
	{
	public:
		void Draw(vk::CommandBuffer commandBuffer, const glm::mat4& transform, const vk::PipelineLayout pipelineLayout);

		void addPrimitive(std::unique_ptr<DDing::Primitive> primitive) { primitives.push_back(std::move(primitive)); }
	private:
		std::vector<std::unique_ptr<DDing::Primitive>> primitives;
	};
}

