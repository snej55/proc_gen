// Created by Jens Kromdijk 05/04/2026

#define VOLK_IMPLEMENTATION
#include <vulkan/vulkan.h>
#include <volk/volk.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <functional>
#include <optional>
#include <set>
#include <limits>

#include "src/constants.hpp"
#include "src/util.hpp"

#define _DEBUG

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

class App
{
public:
    App() = default;
    ~App() = default;

    void run()
    {
        initWindow();
        initVulkan();

        mainLoop();

        free();
    }

private:
    GLFWwindow* m_window{nullptr};

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
    VkSwapchainKHR m_swapChain{VK_NULL_HANDLE};

    // initialize GLFW
    void initWindow()
    {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        m_window = glfwCreateWindow(CST::WIDTH, CST::HEIGHT, "Vulkan Window", nullptr, nullptr);
        if (m_window == nullptr)
        {
            std::cout << BEGIN_ERROR << "APP::INIT_WINDOW::ERROR: Error creating glfw window!" << END_ERROR << std::endl;
            exit(-1);
        }
    }

    // vulkan initialization
    void initVulkan()
    {
        std::cout << "Initializing vulkan..." << std::endl;
        // initialize volk
        volkInitialize();

        createInstance();
        std::cout << "Created instance..." << std::endl;
        createSurface();

        selectPhysicalDevice();
        createLogicalDevice();
        volkLoadDevice(m_device);

        createSwapChain();

        std::cout << "Initialized vulkan!" << std::endl;
    }

    void createInstance()
    {

        // get validation layers and extensions
        std::vector<std::string> validationLayers{};
#ifdef _DEBUG
        for (std::size_t i{0}; i < CST::validationLayers.size(); ++i)
        {
            validationLayers.emplace_back(CST::validationLayers[i]);
        }
#endif

        uint32_t glfwExtensionCount{0};
        const char** glfwExtensions{glfwGetRequiredInstanceExtensions(&glfwExtensionCount)};

        std::vector<std::string> requiredExtensions(static_cast<std::size_t>(glfwExtensionCount));
        for (std::size_t i{0}; i < static_cast<std::size_t>(glfwExtensionCount); ++i)
        {
            requiredExtensions[i] = glfwExtensions[i];
        }

#ifdef _DEBUG
        requiredExtensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

        requiredExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);

        m_enabledInstanceLayers = Util::filterExtensions(enumerateInstanceLayers(), validationLayers);
        m_enabledInstanceExtensions = Util::filterExtensions(enumerateInstanceExtensions(), requiredExtensions);

#ifdef _DEBUG
        std::cout << "Enabled layers: " << std::endl;
        for (const std::string& layer : m_enabledInstanceLayers)
        {
            std::cout << "\t" << layer << std::endl;
        }

        std::cout << "Enabled extensions: " << std::endl;
        for (const std::string& extension : m_enabledInstanceExtensions)
        {
            std::cout << "\t" << extension << std::endl;
        }
#endif

        std::vector<const char*> instanceLayers(m_enabledInstanceLayers.size());
        std::transform(m_enabledInstanceLayers.begin(), m_enabledInstanceLayers.end(), instanceLayers.begin(), std::mem_fn(&std::string::c_str));

        std::vector<const char*> instanceExtensions(m_enabledInstanceExtensions.size());
        std::transform(m_enabledInstanceExtensions.begin(), m_enabledInstanceExtensions.end(), instanceExtensions.begin(), std::mem_fn(&std::string::c_str));

        const VkApplicationInfo appInfo{
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO, .pApplicationName = "Vulkan Window", .applicationVersion = VK_MAKE_VERSION(1, 0, 0), .apiVersion = VK_API_VERSION_1_3};

        VkInstanceCreateInfo instanceCI{
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo = &appInfo,
            .enabledLayerCount = static_cast<uint32_t>(instanceLayers.size()),
            .ppEnabledLayerNames = instanceLayers.data(),
            .enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size()),
            .ppEnabledExtensionNames = instanceExtensions.data()};

        VkDebugUtilsMessengerCreateInfoEXT debugCI{};
#ifdef _DEBUG
        setupDebugMessenger(debugCI);
        instanceCI.pNext = &debugCI;
#else
        instanceCI.pNext = nullptr;
#endif

        instanceCI.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

        VK_CHECK(vkCreateInstance(&instanceCI, nullptr, &m_instance));

        volkLoadInstance(m_instance);

#ifdef _DEBUG
        VK_CHECK(vkCreateDebugUtilsMessengerEXT(m_instance, &debugCI, nullptr, &m_debugMessenger));
#endif
    }

    std::vector<std::string> enumerateInstanceLayers()
    {
        uint32_t instanceLayerCount{0};
        VK_CHECK(vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr));
        std::vector<VkLayerProperties> layers(instanceLayerCount);
        VK_CHECK(vkEnumerateInstanceLayerProperties(&instanceLayerCount, layers.data()));

        std::vector<std::string> availableLayers;
        std::transform(layers.begin(), layers.end(), std::back_inserter(availableLayers), [](const VkLayerProperties& properties) { return properties.layerName; });

#ifdef _DEBUG
        std::cout << "Found " << instanceLayerCount << " available layers(s)" << std::endl;
        for (const std::string& layer : availableLayers)
        {
            std::cout << "\t" << layer << std::endl;
        }
#endif
        return availableLayers;
    }

    std::vector<std::string> enumerateInstanceExtensions(const bool printEnumerations = false)
    {
        uint32_t instanceExtensionCount{0};
        VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, nullptr));
        std::vector<VkExtensionProperties> extensions(instanceExtensionCount);
        VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, extensions.data()));

        std::vector<std::string> availableExtensions;
        std::transform(extensions.begin(), extensions.end(), std::back_inserter(availableExtensions), [](const VkExtensionProperties& properties) { return properties.extensionName; });

#ifdef _DEBUG
        std::cout << "Found " << instanceExtensionCount << " available extension(s)" << std::endl;
        for (const std::string& extension : availableExtensions)
        {
            std::cout << "\t" << extension << std::endl;
        }
#endif
        return availableExtensions;
    }

    void createSurface()
    {
        std::cout << "Created surface..." << std::endl;
        VK_CHECK(glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface));
    }

    [[nodiscard]] bool deviceSuitable(VkPhysicalDevice device) const
    {
        std::cout << "Checking if device is suitable..." << std::endl;
        QueueFamilyIndices indices{findQueueFamilies(device)};

        const bool extensionsSupported(checkDeviceExtensionsSupport(device));

        bool swapChainValid{false};
        if (extensionsSupported)
        {
            SwapChainSupportDetails details{checkSwapChainSupport(device)};
            swapChainValid = !details.formats.empty() && !details.presentModes.empty();
        }
        return indices.complete() && extensionsSupported && swapChainValid;
    }

    [[nodiscard]] bool checkDeviceExtensionsSupport(VkPhysicalDevice device) const
    {
        uint32_t extensionCount;
        VK_CHECK(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr));

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        VK_CHECK(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data()));

        std::set<std::string> requiredExtensions(CST::deviceExtensions.begin(), CST::deviceExtensions.end());

        for (const VkExtensionProperties& extension : availableExtensions)
        {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    void createSwapChain()
    {
        SwapChainSupportDetails details{checkSwapChainSupport(m_physicalDevice)};

        VkSurfaceFormatKHR surfaceFormat{selectSwapChainSurfaceFormat(details.formats)};
        VkPresentModeKHR presentMode{selectSwapChainPresentMode(details.presentModes)};
        VkExtent2D extent{selectSwapExtent(details.capabilities)};

        uint32_t imageCount{details.capabilities.minImageCount + 1};
        if (details.capabilities.maxImageCount > 0 && imageCount > details.capabilities.maxImageCount)
        {
            imageCount = details.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = m_surface;
        // image details
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1; // amount of layers each image has
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices{findQueueFamilies(m_physicalDevice)};
        // must survive longer so not in condition scope
        uint32_t queueFamilyIndices[]{indices.graphicsFamily.value(), indices.presentFamily.value()};

        if (indices.graphicsFamily != indices.presentFamily)
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices = nullptr;
        }

        createInfo.preTransform = details.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        VK_CHECK(vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapChain));

        std::cout << "Created swap chain: " << extent.width << " * " << extent.height << std::endl;
    }

    [[nodiscard]] SwapChainSupportDetails checkSwapChainSupport(VkPhysicalDevice device) const
    {
        SwapChainSupportDetails details;

        VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &details.capabilities));

        uint32_t formatCount;
        VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, nullptr));

        if (formatCount != 0)
        {
            details.formats.resize(formatCount);
            VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, details.formats.data()));
        }

        uint32_t presentModeCount;
        VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, nullptr));

        if (presentModeCount != 0)
        {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    [[nodiscard]] VkSurfaceFormatKHR selectSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats)
    {
        for (const VkSurfaceFormatKHR& format : formats)
        {
            if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                return format;
            }
        }

        // first one is usually fine
        return formats[0];
    }

    [[nodiscard]] VkPresentModeKHR selectSwapChainPresentMode(const std::vector<VkPresentModeKHR>& presentModes) const
    {
        for (const VkPresentModeKHR& presentMode : presentModes)
        {
            if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) // ooh yea we like triple buffering
            {
                return presentMode;
            }
        }
        return VK_PRESENT_MODE_FIFO_KHR; // only one guaranteed to be available
    }

    [[nodiscard]] VkExtent2D selectSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        {
            return capabilities.currentExtent;
        }
        int width, height;
        glfwGetFramebufferSize(m_window, &width, &height);

        VkExtent2D extent{static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

        extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return extent;
    }

    void selectPhysicalDevice()
    {
        std::cout << "Selecting physical device..." << std::endl;
        uint32_t physicalDeviceCount{0};
        VK_CHECK(vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, nullptr));
        CHECK((physicalDeviceCount != 0));
        std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
        VK_CHECK(vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, physicalDevices.data()));

        for (const VkPhysicalDevice& device : physicalDevices)
        {
            if (deviceSuitable(device))
            {
                m_physicalDevice = device;
                break;
            }
        }

        CHECK((m_physicalDevice != VK_NULL_HANDLE));

#ifdef _DEBUG
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(m_physicalDevice, &deviceProperties);
        std::cout << "Selected physical device:" << std::endl;
        std::cout << "\t" << deviceProperties.deviceName << std::endl;
#endif
    }

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) const
    {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount{0};
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        std::cout << "Got queue families..." << std::endl;

        int i{0};
        for (const VkQueueFamilyProperties& queueFamily : queueFamilies)
        {
            if (queueFamily.queueFlags | VK_QUEUE_GRAPHICS_BIT)
            {
                indices.graphicsFamily = i;
            }
            std::cout << "Checked graphics bit..." << std::endl;

            VkBool32 presentSupport{false};
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &presentSupport);
            if (presentSupport)
            {
                indices.presentFamily = i;
            }
            std::cout << "Checked presentation support..." << std::endl;

            if (indices.complete())
            {
                break;
            }
            ++i;
        }

        return indices;
    }

    void createLogicalDevice()
    {
        std::cout << "Creating logical device..." << std::endl;
        QueueFamilyIndices indices{findQueueFamilies(m_physicalDevice)};

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
        std::set<uint32_t> uniqueQueueFamiles{indices.graphicsFamily.value(), indices.presentFamily.value()};

        float queuePriority{1.0f};
        for (uint32_t queueFamily : uniqueQueueFamiles)
        {
            VkDeviceQueueCreateInfo queueCI{.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, .queueFamilyIndex = queueFamily, .queueCount = 1, .pQueuePriorities = &queuePriority};
            queueCreateInfos.push_back(queueCI);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};

        // actual logical device
        VkDeviceCreateInfo createInfo{
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
            .pQueueCreateInfos = queueCreateInfos.data(),
            .pEnabledFeatures = &deviceFeatures};

        createInfo.enabledExtensionCount = static_cast<uint32_t>(CST::deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = CST::deviceExtensions.data();
#ifdef _DEBUG
        std::vector<const char*> instanceLayers(m_enabledInstanceLayers.size());
        std::transform(m_enabledInstanceLayers.begin(), m_enabledInstanceLayers.end(), instanceLayers.begin(), std::mem_fn(&std::string::c_str));
        createInfo.enabledLayerCount = static_cast<uint32_t>(instanceLayers.size());
        createInfo.ppEnabledLayerNames = instanceLayers.data();
#else
        createInfo.enabledLayerCount = 0;
#endif

        std::cout << "Creating logical device..." << std::endl;
        VK_CHECK(vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device));
        vkGetDeviceQueue(m_device, indices.graphicsFamily.value(), 0, &m_graphicsQueue);
        vkGetDeviceQueue(m_device, indices.presentFamily.value(), 0, &m_presentQueue);
    }

    // ------- Debug callback ------- //
    static VKAPI_ATTR VkBool32 VKAPI_CALL
    debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
    {
        std::string colorCode{BEGIN_LOG};
        if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        {
            colorCode = BEGIN_ERROR;
        }
        else if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
            colorCode = BEGIN_WARNING;
        }
        std::cerr << colorCode << "Validation layer: " << pCallbackData->pMessage << END_LOG << std::endl;
        return VK_FALSE;
    }

    void setupDebugMessenger(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
    {
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
        createInfo.pUserData = nullptr;
    }
    // ------------------------------ //

    void mainLoop()
    {
        glfwPollEvents();
        /*        while (!glfwWindowShouldClose(m_window))
                {
                    glfwPollEvents();
                }
        */
    }

    // cleanup resources
    void free()
    {
#ifdef _DEBUG
        if (m_debugMessenger != VK_NULL_HANDLE)
        {
            vkDestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
        }
#endif
        vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);
        vkDestroyDevice(m_device, nullptr);
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        vkDestroyInstance(m_instance, nullptr);
        glfwDestroyWindow(m_window);
        glfwTerminate();
        std::cout << "Cleaned up!" << std::endl;
    }
};

int main()
{
    App app{};

    try
    {
        app.run();
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "We ran!" << std::endl;

    return EXIT_SUCCESS;
}
