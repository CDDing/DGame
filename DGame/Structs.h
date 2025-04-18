#pragma once
#include <vma/vk_mem_alloc.h>
struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails {
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
};

namespace DDing {
    class Context;
    struct Image {
        static void setImageLayout(vk::CommandBuffer commandBuffer, vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
        Image() = default;
        Image(VkImageCreateInfo imgInfo, VmaAllocationCreateInfo allocInfo, vk::ImageViewCreateInfo imageViewInfo);

        //non-copyable
        Image(const Image& other) = delete;
        Image& operator=(const Image& other) = delete;

        //Only movable
        Image(Image&& other) noexcept {
            *this = std::move(other);
        };
        Image& operator=(Image&& other) noexcept {
            image = other.image;
            imageView = other.imageView;
            allocation = other.allocation;
            layout = other.layout;
            format = other.format;
            mipLevel = other.mipLevel;

            other.image = nullptr;
            other.imageView = nullptr;
            other.allocation = nullptr;
            other.layout = vk::ImageLayout::eUndefined;
            other.format = {};
            other.mipLevel = 0;
            return *this;
        };

        ~Image();
        void setImageLayout(vk::CommandBuffer commandBuffer, vk::ImageLayout newLayout);
        VkImage image = nullptr;
        vk::ImageView imageView = nullptr;
        VmaAllocation allocation;

        vk::ImageLayout layout = vk::ImageLayout::eUndefined;
        vk::Format format;
        uint32_t mipLevel;
    };
    struct Buffer {
        vk::Buffer buffer = nullptr;
    };
}