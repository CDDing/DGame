#pragma once
#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif
namespace DDing {
	class Context
	{
		friend class SwapChain;
	private:
		//For Initailize Order
		std::vector<vk::Queue> queues;
		GLFWwindow* window;
	public:
		enum QueueType {
			GRAPHICS,
			PRESENT,
			END
		};

		Context(GLFWwindow* window);
		~Context() {};

		vk::Queue& GetQueue(QueueType type) { return queues[static_cast<int>(type)]; }
		vk::Queue& GetQueue(int type) { return queues[type]; }

		vk::raii::Context context;
		vk::raii::Instance instance;
		vk::raii::DebugUtilsMessengerEXT debug;
		vk::raii::SurfaceKHR surface;
		vk::raii::PhysicalDevice physical;
		vk::raii::Device logical;
	private:
		vk::raii::Instance createInstance();
		vk::raii::DebugUtilsMessengerEXT createDebugMessenger();
		vk::raii::SurfaceKHR createSurface();
		vk::raii::PhysicalDevice createPhysicalDevice();
		vk::raii::Device createLogicalDevice();

		bool checkValidationLayerSupport();
		std::vector<const char*> getRequiredExtensions();
		vk::DebugUtilsMessengerCreateInfoEXT createDebugMessengerInfo();
		bool isDeviceSuitable(vk::PhysicalDevice device, vk::SurfaceKHR surface);
		QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice device, vk::SurfaceKHR surface);
	};


}
const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation",
};
const std::vector<const char*> instanceExtensions = {
	vk::KHRSurfaceExtensionName,
};
const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
	VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,

	VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
	VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
	VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,

	VK_KHR_SPIRV_1_4_EXTENSION_NAME,

	VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME,


};
VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT       messageSeverity,
	vk::DebugUtilsMessageTypeFlagsEXT              messageTypes,
	vk::DebugUtilsMessengerCallbackDataEXT const* pCallbackData,
	void* /*pUserData*/);