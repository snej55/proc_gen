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

#include "src/constants.hpp"
#include "src/util.hpp"

#define _DEBUG

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
    std::unordered_set<std::string> m_enabledLayers{};
    std::unordered_set<std::string> m_enabledInstanceExtensions{};

    VkInstance m_instance{VK_NULL_HANDLE};

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
        // initialize volk
        volkInitialize();

        // get validation layers and extensions
        std::vector<std::string> validationLayers{};
#ifdef _DEBUG
        validationLayers.emplace_back("VK_LAYER_KHRONOS_validation");
#endif

        std::vector<std::string> requestedInstanceExtensions{
#if defined(VK_KHR_win32_surface)
            VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
#if defined(VK_EXT_debug_utils)
            VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
#if defined(VK_KHR_surface)
            VK_KHR_SURFACE_EXTENSION_NAME,
#endif
        };
        m_enabledLayers = Util::filterExtensions(enumerateInstanceLayers(), validationLayers);
        m_enabledInstanceExtensions = Util::filterExtensions(enumerateInstanceExtensions(), requestedInstanceExtensions);

#ifdef _DEBUG
        std::cout << "Enabled layers: " << std::endl;
        for (const std::string& layer : m_enabledLayers)
        {
            std::cout << "\t" << layer << std::endl;
        }

        std::cout << "Enabled extensions: " << std::endl;
        for (const std::string& extension : m_enabledInstanceExtensions)
        {
            std::cout << "\t" << extension << std::endl;
        }
#endif

        std::vector<const char*> instanceLayers(m_enabledLayers.size());
        std::transform(m_enabledLayers.begin(), m_enabledLayers.end(), instanceLayers.begin(), std::mem_fn(&std::string::c_str));

        std::vector<const char*> instanceExtensions(m_enabledInstanceExtensions.size());
        std::transform(m_enabledInstanceExtensions.begin(), m_enabledInstanceExtensions.end(), instanceExtensions.begin(), std::mem_fn(&std::string::c_str));

        // create Vulkan instance
        const VkApplicationInfo appInfo{
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO, .pApplicationName = "Vulkan Window", .applicationVersion = VK_MAKE_VERSION(1, 0, 0), .apiVersion = VK_API_VERSION_1_3};

        const VkInstanceCreateInfo instanceCI{
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo = &appInfo,
            .enabledLayerCount = static_cast<uint32_t>(instanceLayers.size()),
            .ppEnabledLayerNames = instanceLayers.data(),
            .enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size()),
            .ppEnabledExtensionNames = instanceExtensions.data()};

        VK_CHECK(vkCreateInstance(&instanceCI, nullptr, &m_instance));

        std::cout << "Initialized vulkan!" << std::endl;
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

    void mainLoop()
    {
        while (!glfwWindowShouldClose(m_window))
        {
            glfwPollEvents();
        }
    }

    // cleanup resources
    void free()
    {
        glfwDestroyWindow(m_window);
        glfwTerminate();
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
