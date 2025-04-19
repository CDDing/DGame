#pragma once
namespace DDing {
	struct Vertex {
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 texcoord;
	};
	class Mesh
	{
	public:
		Mesh();
		Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices);

	private:
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		DDing::Buffer vertexBuffer;
		DDing::Buffer indexBuffer;

		vk::DeviceAddress vertexBufferAddress = 0;
	};
}

