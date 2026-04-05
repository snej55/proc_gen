// Created by Jens Kromdijk 05/04/2026

#ifndef UTIL_H
#define UTIL_H

#include <cstdlib>
#include <iostream>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#define BEGIN_ERROR "\033[41m"
#define END_ERROR "\033[m"

namespace Util
{
    inline void check(VkResult result)
    {
        if (result != VK_SUCCESS)
        {
            std::cerr << BEGIN_ERROR << "Vulkan call return error (" << result << ")" << END_ERROR << std::endl;
            exit(result);
        }
    }

    inline void checkSwapchain(VkResult result, bool* updateSwapchain)
    {
        if (result < VK_SUCCESS)
        {
            if (result == VK_ERROR_OUT_OF_DATE_KHR)
            {
                *updateSwapchain = true;
                return;
            }

            std::cerr << BEGIN_ERROR << "Vulkan call return error (" << result << ")" << END_ERROR << std::endl;
            exit(result);
        }
    }

    inline void check(const bool result)
    {
        if (!result)
        {
            std::cerr << BEGIN_ERROR << "Call returned an error!" << END_ERROR << std::endl;
            exit(result);
        }
    }
} // namespace Util

#endif // UTIL_H
