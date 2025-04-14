#include "pch.h"
#include "Context.h"

DDing::Context::Context(GLFWwindow* window)
	:
	window(window),
	context(),
	instance(createInstance()),
	debug(createDebugMessenger()),
	surface(createSurface()),
	physical(createPhysicalDevice()),
	logical(createLogicalDevice())
{
}

vk::raii::Instance DDing::Context::createInstance()
{
	if(enableValidationLayers && !checkValidationLayerSupport())
		throw std::runtime_error("validation layers requested, but not available!");

	vk::ApplicationInfo appInfo{ "DDing",
	VK_MAKE_VERSION(1,0,0),
	"No Engine",
	VK_MAKE_VERSION(1,0,0),
	VK_API_VERSION_1_3 };

	vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo = createDebugMessengerInfo();
	vk::InstanceCreateInfo createInfo{};
	createInfo.setPApplicationInfo(&appInfo);
	
	if (enableValidationLayers) {
		createInfo.setPEnabledLayerNames(validationLayers);
		createInfo.pNext = &debugCreateInfo;
	}

	auto extensions = getRequiredExtensions();
	createInfo.setPEnabledExtensionNames(extensions);


	return vk::raii::Instance(context,createInfo);
}

vk::raii::DebugUtilsMessengerEXT DDing::Context::createDebugMessenger()
{
	if (!enableValidationLayers) return VK_NULL_HANDLE;
	vk::DebugUtilsMessengerCreateInfoEXT createInfo = createDebugMessengerInfo();
	return instance.createDebugUtilsMessengerEXT(createInfo);
}

vk::raii::SurfaceKHR DDing::Context::createSurface()
{
	VkSurfaceKHR surface;
	VkResult result = glfwCreateWindowSurface(static_cast<VkInstance>(*instance), window, nullptr, &surface);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to create window surface!");
	}

	return vk::raii::SurfaceKHR(instance, surface);
}

vk::raii::PhysicalDevice DDing::Context::createPhysicalDevice()
{
	vk::raii::PhysicalDevices devices(instance);
	for (const auto& device : devices) {
		if (isDeviceSuitable(device, surface)) {
			return device;
			break;
		}
	}

	if (physical == VK_NULL_HANDLE) {
		throw std::runtime_error("failed to find a suitable GPU!");
	}

	return vk::raii::PhysicalDevice(nullptr);
}

vk::raii::Device DDing::Context::createLogicalDevice()
{
	QueueFamilyIndices indices = findQueueFamilies(physical, surface);

	std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(),indices.presentFamily.value() };

	float queuePriority = 1.f;
	for (uint32_t queueFamiliy : uniqueQueueFamilies) {
		vk::DeviceQueueCreateInfo queueCreateInfo{ {},queueFamiliy,1, &queuePriority };
		queueCreateInfos.push_back(queueCreateInfo);
	}
	//For RT
	vk::PhysicalDeviceDescriptorIndexingFeaturesEXT pddifEXT;
	pddifEXT.runtimeDescriptorArray = VK_TRUE;

	vk::PhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeatures{};
	rayTracingPipelineFeatures.rayTracingPipeline = VK_TRUE;  // 기능 활성화
	rayTracingPipelineFeatures.pNext = &pddifEXT;

	VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures = {};
	accelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
	accelerationStructureFeatures.accelerationStructure = VK_TRUE;
	VkPhysicalDeviceBufferDeviceAddressFeatures bufferDeviceAddressFeatures = {};
	bufferDeviceAddressFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
	bufferDeviceAddressFeatures.bufferDeviceAddress = VK_TRUE;
	bufferDeviceAddressFeatures.pNext = &rayTracingPipelineFeatures;

	accelerationStructureFeatures.pNext = &bufferDeviceAddressFeatures;

	VkPhysicalDeviceFeatures2 deviceFeatures{};
	deviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	deviceFeatures.features.samplerAnisotropy = VK_TRUE;
	deviceFeatures.features.shaderInt64 = VK_TRUE;
	deviceFeatures.pNext = &accelerationStructureFeatures;

	std::vector<const char*> validationlayers;
	if (enableValidationLayers) {
		validationlayers = validationLayers;
	}
	vk::DeviceCreateInfo createInfo{ {},queueCreateInfos,validationLayers,deviceExtensions };
	createInfo.pNext = &deviceFeatures;

	vk::raii::Device logicalDevice = physical.createDevice(createInfo);

	for (int i = 0; i < QueueType::END; i++) {
		vk::Queue q;
		queues.push_back(q);
	}

	GetQueue(QueueType::GRAPHICS) = logicalDevice.getQueue(indices.graphicsFamily.value(), 0);
	GetQueue(QueueType::PRESENT) = logicalDevice.getQueue(indices.presentFamily.value(), 0);
	return logicalDevice;
}

bool DDing::Context::checkValidationLayerSupport()
{
	std::vector<vk::LayerProperties> availableLayers = vk::enumerateInstanceLayerProperties();

	for (const char* layerName : validationLayers) {
		bool layerFound = false;
		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}
		if (!layerFound) {
			return false;
		}
	}
	return true;
}

std::vector<const char*> DDing::Context::getRequiredExtensions()
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (enableValidationLayers) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

vk::DebugUtilsMessengerCreateInfoEXT DDing::Context::createDebugMessengerInfo()
{
	vk::DebugUtilsMessengerCreateInfoEXT createInfo;
	createInfo.messageSeverity =
		vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
		vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
		vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
	createInfo.messageType =
		vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
		vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
		vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
	createInfo.pfnUserCallback = debugCallback;
	return createInfo;
}

bool DDing::Context::isDeviceSuitable(vk::PhysicalDevice device, vk::SurfaceKHR surface)
{

    QueueFamilyIndices indices = findQueueFamilies(device, surface);

    bool extensionsSupported = true;//checkDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = SwapChain::querySwapChainSupport(device, surface);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    vk::PhysicalDeviceFeatures supportedFeatures = device.getFeatures();
    return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

QueueFamilyIndices DDing::Context::findQueueFamilies(vk::PhysicalDevice device, vk::SurfaceKHR surface)
{
	QueueFamilyIndices indices;

	std::vector<vk::QueueFamilyProperties> queueFamilies =
		device.getQueueFamilyProperties();;

	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		if (queueFamily.queueFlags && VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		}
		VkBool32 presentSupport = device.getSurfaceSupportKHR(i, surface);
		if (presentSupport) {
			indices.presentFamily = i;
		}
		if (indices.isComplete()) {
			break;
		}
		i++;
	}
	return indices;

}

VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity, vk::DebugUtilsMessageTypeFlagsEXT messageTypes, vk::DebugUtilsMessengerCallbackDataEXT const* pCallbackData, void*)
{
	std::ostringstream message;

	message << vk::to_string(messageSeverity) << ": " << vk::to_string(messageTypes) << ":\n";
	message << std::string("\t") << "messageIDName   = <" << pCallbackData->pMessageIdName << ">\n";
	message << std::string("\t") << "messageIdNumber = " << pCallbackData->messageIdNumber << "\n";
	message << std::string("\t") << "message         = <" << pCallbackData->pMessage << ">\n";
	if (0 < pCallbackData->queueLabelCount)
	{
		message << std::string("\t") << "Queue Labels:\n";
		for (uint32_t i = 0; i < pCallbackData->queueLabelCount; i++)
		{
			message << std::string("\t\t") << "labelName = <" << pCallbackData->pQueueLabels[i].pLabelName << ">\n";
		}
	}
	if (0 < pCallbackData->cmdBufLabelCount)
	{
		message << std::string("\t") << "CommandBuffer Labels:\n";
		for (uint32_t i = 0; i < pCallbackData->cmdBufLabelCount; i++)
		{
			message << std::string("\t\t") << "labelName = <" << pCallbackData->pCmdBufLabels[i].pLabelName << ">\n";
		}
	}
	if (0 < pCallbackData->objectCount)
	{
		message << std::string("\t") << "Objects:\n";
		for (uint32_t i = 0; i < pCallbackData->objectCount; i++)
		{
			message << std::string("\t\t") << "Object " << i << "\n";
			message << std::string("\t\t\t") << "objectType   = " << vk::to_string(pCallbackData->pObjects[i].objectType) << "\n";
			message << std::string("\t\t\t") << "objectHandle = " << pCallbackData->pObjects[i].objectHandle << "\n";
			if (pCallbackData->pObjects[i].pObjectName)
			{
				message << std::string("\t\t\t") << "objectName   = <" << pCallbackData->pObjects[i].pObjectName << ">\n";
			}
		}
	}
	std::cout << message.str() << std::endl;
	return vk::False;
}
