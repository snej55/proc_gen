// Created by Jens Kromdijk 05/04/2026

#include <cstdlib>
#include <iostream>
#include <vector>

#define VOLK_IMPLEMENTATION
#include <vulkan/vulkan.h>
#include <volk/volk.h>

#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "src/util.hpp"

int main(int argc, char* argv[])
{
    VkInstance instance{VK_NULL_HANDLE};
    VkDevice device{VK_NULL_HANDLE};
    VkQueue queue{VK_NULL_HANDLE};
    VmaAllocator allocator{VK_NULL_HANDLE};
    VkSurfaceKHR surface{VK_NULL_HANDLE};

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cout << BEGIN_ERROR << "INIT::ERROR: Failed to initialize SDL3!" << END_ERROR << std::endl;
        return EXIT_FAILURE;
    }

    if (!SDL_Vulkan_LoadLibrary(nullptr))
    {
        std::cout << BEGIN_ERROR << "INIT::ERROR: Failed to load SDL vulkan library!" << END_ERROR << std::endl;
        return EXIT_FAILURE;
    }

    volkInitialize();

    VkApplicationInfo appInfo{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Proc Gen",
        .apiVersion = VK_API_VERSION_1_3,
    };

    uint32_t instanceExtensionsCount{0};
    char const* const* instanceExtensions{SDL_Vulkan_GetInstanceExtensions(&instanceExtensionsCount)};

    std::vector<const char*> extensions(instanceExtensions, instanceExtensions + instanceExtensionsCount);
    extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

    std::cout << "Extensions (" << extensions.size() << "):\n";
    for (std::size_t i{0}; i < extensions.size(); ++i)
    {
        std::cout << "\t" << extensions[i] << "\n";
    }
    std::cout << std::endl;

    VkInstanceCreateInfo instanceCI{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
        .pApplicationInfo = &appInfo,
        .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data()};
    if (vkCreateInstance(&instanceCI, nullptr, &instance) != VK_SUCCESS)
    {
        std::cout << BEGIN_ERROR << "INIT::ERROR: Failed to create Vulkan instance!" << END_ERROR << std::endl;
        return EXIT_FAILURE;
    }
    volkLoadInstance(instance);

    uint32_t deviceCount{0};
    if (vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr) != VK_SUCCESS)
    {
        std::cout << BEGIN_ERROR << "INIT::ERROR: Failed to list available physical devices!" << END_ERROR << std::endl;
        return EXIT_FAILURE;
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    if (vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data()) != VK_SUCCESS)
    {
        std::cout << BEGIN_ERROR << "INIT::ERROR: Failed to list available physical devices!" << END_ERROR << std::endl;
        return EXIT_FAILURE;
    }

    uint32_t deviceIndex{0};
    if (argc > 1)
    {
        deviceIndex = std::stoi(argv[1]);
        assert(deviceIndex < deviceCount);
    }

    VkPhysicalDeviceProperties2 deviceProperties{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
    vkGetPhysicalDeviceProperties2(devices[deviceIndex], &deviceProperties);
    std::cout << "Selected device: " << deviceProperties.properties.deviceName << std::endl;

    uint32_t queueFamilyCount{0};
    vkGetPhysicalDeviceQueueFamilyProperties(devices[deviceIndex], &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(devices[deviceIndex], &queueFamilyCount, queueFamilies.data());
    uint32_t queueFamily{0};
    for (std::size_t i{0}; i < queueFamilies.size(); ++i)
    {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            queueFamily = i;
            break;
        }
    }

    if (!SDL_Vulkan_GetPresentationSupport(instance, devices[deviceIndex], queueFamily))
    {
        std::cout << BEGIN_ERROR << "INIT::ERROR: Selected device does not support presentation!" << END_ERROR
                  << std::endl;
        return EXIT_FAILURE;
    }

    const float qfpriorities{1.0f};
    VkDeviceQueueCreateInfo queueCI{
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = queueFamily,
        .queueCount = 1,
        .pQueuePriorities = &qfpriorities};

    VkPhysicalDeviceVulkan12Features enabledVk12Features{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
        .descriptorIndexing = true,
        .shaderSampledImageArrayNonUniformIndexing = true,
        .descriptorBindingVariableDescriptorCount = true,
        .runtimeDescriptorArray = true,
        .bufferDeviceAddress = true};
    VkPhysicalDeviceVulkan13Features enabledVk13Features{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .pNext = &enabledVk12Features,
        .synchronization2 = true,
        .dynamicRendering = true};
    VkPhysicalDeviceFeatures enabledVk10Features{.samplerAnisotropy = VK_TRUE};

    uint32_t extensionsCount = 0;
    vkEnumerateDeviceExtensionProperties(devices[deviceIndex], nullptr, &extensionsCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extensionsCount);
    vkEnumerateDeviceExtensionProperties(devices[deviceIndex], nullptr, &extensionsCount, availableExtensions.data());

    std::vector<const char*> deviceExtensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    for (const VkExtensionProperties& extension : availableExtensions)
    {
        if (std::string(extension.extensionName) == "VK_KHR_portability_subset")
        {
            deviceExtensions.push_back("VK_KHR_portability_subset");
            break;
        }
    }

    VkDeviceCreateInfo deviceCI{
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &enabledVk13Features,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queueCI,
        .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
        .ppEnabledExtensionNames = deviceExtensions.data(),
        .pEnabledFeatures = &enabledVk10Features};

    if (vkCreateDevice(devices[deviceIndex], &deviceCI, nullptr, &device) != VK_SUCCESS)
    {
        std::cout << BEGIN_ERROR << "INIT::ERROR: Failed to create Vulkan device!" << END_ERROR << std::endl;
        return EXIT_FAILURE;
    }

    vkGetDeviceQueue(device, queueFamily, 0, &queue);

    // setup VMA
    VmaVulkanFunctions vkFunctions{
        .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
        .vkGetDeviceProcAddr = vkGetDeviceProcAddr,
        .vkCreateImage = vkCreateImage};

    VmaAllocatorCreateInfo allocatorCI{
        .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
        .physicalDevice = devices[deviceIndex],
        .device = device,
        .pVulkanFunctions = &vkFunctions,
        .instance = instance};

    if (vmaCreateAllocator(&allocatorCI, &allocator) != VK_SUCCESS)
    {
        std::cout << BEGIN_ERROR << "INIT::ERROR: Failed to create VMA allocator!" << END_ERROR << std::endl;
        return EXIT_FAILURE;
    }

    // setup SDL window and surfaces
    SDL_Window* window{SDL_CreateWindow("SDL x Vulkan", 1280, 720, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE)};
    if (!SDL_Vulkan_CreateSurface(window, instance, nullptr, &surface))
    {
    }

    std::cout << "We ran!" << std::endl;
    return EXIT_SUCCESS;
}
