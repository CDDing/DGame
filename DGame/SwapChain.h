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

        int imgCnt;
        vk::Format imageFormat;
        vk::Format depthFormat;
    private:
        std::vector<vk::Image> images;
        std::vector<vk::raii::ImageView> imageViews;
        vk::raii::SwapchainKHR swapChain;
        DDing::Context* context;

    };

}