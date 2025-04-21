#pragma once
namespace DDing {
	class Camera;
}
class InputManager
{
public:
	~InputManager();

	void Update();
	void Init();
	void DrawImGui(vk::CommandBuffer commandBuffer, uint32_t imageIndex);
private:
	void createImGuiResources();
	void DrawSceneHierarchy(DDing::GameObject* gameObject);

	vk::raii::RenderPass renderPass = nullptr;
	vk::raii::Pipeline pipeline = nullptr;
	vk::raii::PipelineLayout pipelineLayout = nullptr;
	vk::raii::PipelineCache pipelineCache = nullptr;
	vk::raii::DescriptorPool descriptorPool = nullptr;
	vk::raii::DescriptorSetLayout descriptorSetLayout = nullptr;
	std::vector<vk::raii::Framebuffer> swapChainFrameBuffers;

	friend class DDing::Camera;
	DDing::GameObject* selectedObject = nullptr;
	static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static bool keyPressed[256];
};

