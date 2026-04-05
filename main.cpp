// Created by Jens Kromdijk 05/04/2026

#include <cstdlib>
#include <iostream>
#include <vector>

#define VOLK_IMPLEMENTATION
#include <vulkan/vulkan.h>
#include <volk/volk.h>

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
    VkDevice queue{VK_NULL_HANDLE};

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

    std::cout << "Extensions (" << instanceExtensionsCount << "):\n";
    for (std::size_t i{0}; i < instanceExtensionsCount; ++i)
    {
        std::cout << "\t" << instanceExtensions[i] << "\n";
    }
    std::cout << std::endl;

    VkInstanceCreateInfo instanceCI{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledExtensionCount = instanceExtensionsCount,
        .ppEnabledExtensionNames = instanceExtensions};
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
    // const std::vector<const char*>

    std::cout << "We ran!" << std::endl;
    return EXIT_SUCCESS;
}
