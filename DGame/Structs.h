#pragma once
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
    struct Image {
        Image() = default;
        //Only movable
        Image(Image&& other) noexcept {
            *this = std::move(other);
        }
        Image& operator=(Image&& other) noexcept {
            if (this == &other)
                return *this;



            return *this;
        }

        //non-copyable
        Image(const Image& other) = delete;
        Image& operator=(const Image& other) = delete;

        vk::Image image = nullptr;
        vk::ImageView imageView = nullptr;
        VmaAllocation allocation;
    };
    struct Buffer {
        vk::Buffer buffer = nullptr;
    };
}