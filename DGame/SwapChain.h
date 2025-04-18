#pragma once
namespace DDing {
    class SwapChain
    {
    public:
        static SwapChainSupportDetails querySwapChainSupport(vk::PhysicalDevice device, vk::SurfaceKHR surface) {
            SwapChainSupportDetails details;
            details.capabilities = device.getSurfaceCapabilitiesKHR(surface);
            details.formats = device.getSurfaceFormatsKHR(surface);
            details.presentModes = device.getSurfacePresentModesKHR(surface);
            return details;
        }
        SwapChain(DDing::Context& context);
        void create();

        vk::raii::SwapchainKHR& Get() { return swapChain; }

        int imgCnt;
        vk::Format imageFormat;
        vk::Extent2D extent;
    private:    
        vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
        vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);
        vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);


        std::vector<vk::Image> images;
        std::vector<vk::raii::ImageView> imageViews;
        vk::raii::SwapchainKHR swapChain;
        DDing::Context* context;

    };

}