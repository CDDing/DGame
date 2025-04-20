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
	class Mesh
	{
	public:
		Mesh();
		Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices);
		void Draw(vk::CommandBuffer commandBuffer);
		
		vk::DeviceAddress vertexBufferAddress = 0;
	private:
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		DDing::Buffer vertexBuffer;
		DDing::Buffer indexBuffer;

	};
}

