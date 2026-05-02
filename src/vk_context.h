// Created by Jens Kromdijk 22/04/2026

#ifndef CONTEXT_H
#define CONTEXT_H

#include <vulkan/vulkan.h>
#include <volk/volk.h>

#include <vma/vk_mem_alloc.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include <fmt/base.h>

#include <vector>
#include <optional>
#include <string>
#include <unordered_set>
#include <array>
#include <span>

#include "engine_types.h"

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool complete() const { return graphicsFamily.has_value() && presentFamily.has_value(); }
};

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct FrameData
{
    VkCommandPool m_commandPool;
    VkCommandBuffer m_commandBuffer;
    VkSemaphore m_swapchainSemaphore;
    VkSemaphore m_renderSemaphore;
    VkFence m_renderFence;
    DeletionQueue m_deletionQueue{};
};

struct AllocatedImage
{
    VkImage m_image;
    VkImageView m_imageView;
    VmaAllocation m_allocation;
    VkExtent3D m_extent;
    VkFormat m_imageFormat;
};

struct DescriptorLayoutBuilder
{
    std::vector<VkDescriptorSetLayoutBinding> m_bindings;

    void addBinding(uint32_t binding, VkDescriptorType type);
    void clear();

    [[nodiscard]] VkDescriptorSetLayout build(VkDevice device, VkShaderStageFlags shaderStages, void* pNext = nullptr, VkDescriptorSetLayoutCreateFlags flags = 0);
};

struct DescriptorAllocator
{
    struct PoolSizeRatio
    {
        VkDescriptorType type;
        float ratio;
    };

    VkDescriptorPool m_pool;

    void initPool(VkDevice device, uint32_t maxSets, std::span<PoolSizeRatio> poolRatios);
    void clearDescriptors(VkDevice device);
    void destroyPool(VkDevice device);

    [[nodiscard]] VkDescriptorSet allocate(VkDevice device, VkDescriptorSetLayout layout);
};

constexpr unsigned int FRAME_OVERLAP{2};

class Context
{
public:
    Context() = default;
    ~Context() = default;

    void init();
    void draw();
    void free();

    void tickFrame();

    [[nodiscard]] SDL_Window* getWindow() { return m_window; }

    [[nodiscard]] const std::unordered_set<std::string>& getEnabledInstanceLayers() const { return m_enabledInstanceLayers; }
    [[nodiscard]] const std::unordered_set<std::string>& getEnabledInstanceExtensions() const { return m_enabledInstanceExtensions; }

    [[nodiscard]] VkDebugUtilsMessengerEXT& getDebugMessenger() { return m_debugMessenger; }

    [[nodiscard]] VkInstance& getInstance() { return m_instance; }
    [[nodiscard]] VkSurfaceKHR& getSurface() { return m_surface; }
    [[nodiscard]] VkPhysicalDevice& getPhysicalDevice() { return m_physicalDevice; }
    [[nodiscard]] VkDevice& getDevice() { return m_device; }

    [[nodiscard]] VkQueue& getGraphicsQueue() { return m_graphicsQueue; }
    [[nodiscard]] VkQueue& getPresentQueue() { return m_presentQueue; }

    [[nodiscard]] VkSwapchainKHR& getSwapchain() { return m_swapchain; }
    [[nodiscard]] const std::vector<VkImage>& getSwapchainImages() const { return m_swapchainImages; }
    [[nodiscard]] VkFormat& getSwapchainImageFormat() { return m_swapchainImageFormat; }
    [[nodiscard]] VkExtent2D& getSwapchainExtent() { return m_swapchainExtent; }
    [[nodiscard]] const std::vector<VkImageView>& getSwapchainImageViews() const { return m_swapchainImageViews; }

    [[nodiscard]] FrameData& getCurrentFrame() { return m_frames[m_frameNumber % FRAME_OVERLAP]; }
    [[nodiscard]] std::size_t getFrameNumber() const { return m_frameNumber; }

    [[nodiscard]] bool getInit() const { return m_init; }

    [[nodiscard]] DeletionQueue& getFrameDeletionQueue() { return getCurrentFrame().m_deletionQueue; }
    [[nodiscard]] DeletionQueue& getMainDeletionQueue() { return m_deletionQueue; }

private:
    SDL_Window* m_window{nullptr};

    // --------- Vulkan components --------- //
    std::unordered_set<std::string> m_enabledInstanceLayers{};
    std::unordered_set<std::string> m_enabledInstanceExtensions{};

    VkDebugUtilsMessengerEXT m_debugMessenger{VK_NULL_HANDLE};

    VkInstance m_instance{VK_NULL_HANDLE};
    VkSurfaceKHR m_surface{VK_NULL_HANDLE};
    VkPhysicalDevice m_physicalDevice{VK_NULL_HANDLE};
    VkDevice m_device{VK_NULL_HANDLE};
    QueueFamilyIndices m_queueFamilyIndices;
    VkQueue m_graphicsQueue{VK_NULL_HANDLE};
    VkQueue m_presentQueue{VK_NULL_HANDLE};
    VkSwapchainKHR m_swapchain{VK_NULL_HANDLE};
    std::vector<VkImage> m_swapchainImages{};
    VkFormat m_swapchainImageFormat{};
    VkExtent2D m_swapchainExtent{};
    std::vector<VkImageView> m_swapchainImageViews{};

    AllocatedImage m_drawImage{};
    VkExtent2D m_drawExtent{};

    VmaAllocator m_allocator{VK_NULL_HANDLE};

    std::array<FrameData, FRAME_OVERLAP> m_frames;
    std::size_t m_frameNumber{0};
    DeletionQueue m_deletionQueue{};

    bool m_init{false};

    // ----------- initialization ----------- //
    void initWindow();
    void initVulkan();

    void createInstance();

    void createSurface();

    void selectPhysicalDevice();

    void createLogicalDevice();

    void createSwapchain();

    void createAllocator();

    [[nodiscard]] std::vector<std::string> enumerateInstanceLayers();
    [[nodiscard]] std::vector<std::string> enumerateInstanceExtensions();

    [[nodiscard]] bool deviceSuitable(VkPhysicalDevice device) const;
    [[nodiscard]] bool checkDeviceExtensionsSupport(VkPhysicalDevice device) const;

    [[nodiscard]] QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) const;

    // swap chain creation
    [[nodiscard]] SwapChainSupportDetails checkSwapchainSupport(VkPhysicalDevice device) const;
    [[nodiscard]] VkSurfaceFormatKHR selectSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats) const;
    [[nodiscard]] VkPresentModeKHR selectSwapchainPresentMode(const std::vector<VkPresentModeKHR>& presentModes) const;
    [[nodiscard]] VkExtent2D selectSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;
    void createImageViews();
    void freeSwapchain();
    void recreateSwapchain();

    // setup command structures
    void initCommands();

    // setup sync structures
    void initSyncStructures();

    // ----------- drawing ----------- //
    void drawBackground(VkCommandBuffer cmd);

    // ----------- validation layers ----------- //
    static VKAPI_ATTR VkBool32 VKAPI_CALL
    debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

    void setupDebugMessenger(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
};

#endif
