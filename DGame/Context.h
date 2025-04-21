#pragma once
#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif
namespace DDing {
	class Context
	{
		struct Immediate {
			vk::raii::Fence fence = nullptr;
			vk::raii::CommandPool commandPool = nullptr;
			vk::raii::CommandBuffer commandBuffer = nullptr;
		};
		friend class SwapChain;
	private:
		//For Initailize Order
		std::vector<vk::Queue> queues;
	public:
		enum QueueType {
			GRAPHICS,
			PRESENT,
			END
		};

		Context(GLFWwindow* window);
		~Context();

		vk::Queue& GetQueue(QueueType type) { return queues[static_cast<int>(type)]; }
		vk::Queue& GetQueue(int type) { return queues[type]; }
		QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice device, vk::SurfaceKHR surface);

		GLFWwindow* window;
		vk::raii::Context context;
		vk::raii::Instance instance;
		vk::raii::DebugUtilsMessengerEXT debug;
		vk::raii::SurfaceKHR surface;
		vk::raii::PhysicalDevice physical;
		vk::raii::Device logical;
		VmaAllocator allocator;

		void immediate_submit(std::function<void(vk::CommandBuffer commandBuffer)>&& function);
	private:
		vk::raii::Instance createInstance();
		vk::raii::DebugUtilsMessengerEXT createDebugMessenger();
		vk::raii::SurfaceKHR createSurface();
		vk::raii::PhysicalDevice createPhysicalDevice();
		vk::raii::Device createLogicalDevice();
		void createVmaAllocator();
		void createImmediateResources();

		bool checkValidationLayerSupport();
		std::vector<const char*> getRequiredExtensions();
		vk::DebugUtilsMessengerCreateInfoEXT createDebugMessengerInfo();
		bool isDeviceSuitable(vk::PhysicalDevice device, vk::SurfaceKHR surface);
		
		Immediate immediate;
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