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

class Context
{
public:
    Context() = default;
    ~Context() = default;

    void init();
    void free();

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

    [[nodiscard]] bool getInit() const { return m_init; }

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
    VkQueue m_graphicsQueue{VK_NULL_HANDLE};
    VkQueue m_presentQueue{VK_NULL_HANDLE};
    VkSwapchainKHR m_swapchain{VK_NULL_HANDLE};
    std::vector<VkImage> m_swapchainImages{};
    VkFormat m_swapchainImageFormat{};
    VkExtent2D m_swapchainExtent{};
    std::vector<VkImageView> m_swapchainImageViews{};

    bool m_init{false};

    void initWindow();
    void initVulkan();

    void createInstance();

    void createSurface();

    void selectPhysicalDevice();

    void createLogicalDevice();

    void createSwapchain();

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

    static VKAPI_ATTR VkBool32 VKAPI_CALL
    debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

    void setupDebugMessenger(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
};

#endif
