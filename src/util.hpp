// Created by Jens Kromdijk 05/04/2026

#ifndef UTIL_H
#define UTIL_H

#include <cstdlib>
#include <iostream>
#include <cassert>
#include <unordered_set>
#include <vector>
#include <algorithm>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#define BEGIN_ERROR "\033[41m"
#define END_ERROR "\033[m"

#define VK_CHECK(func)                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        const VkResult result{func};                                                                                                                                                                   \
        if (result != VK_SUCCESS)                                                                                                                                                                      \
        {                                                                                                                                                                                              \
            std::cerr << BEGIN_ERROR << "Error calling function " << #func << " at " << __FILE__ << ":" << __LINE__ << ". Result: (" << result << ")" << END_ERROR << std::endl;                       \
            assert(false);                                                                                                                                                                             \
        }                                                                                                                                                                                              \
    }

#define CHECK(func)                                                                                                                                                                                    \
    {                                                                                                                                                                                                  \
        if (!func)                                                                                                                                                                                     \
        {                                                                                                                                                                                              \
            std::cerr << BEGIN_ERROR << "Error calling function " << #func << " at " << __FILE__ << ":" << __LINE__ << END_ERROR << std::endl;                                                         \
            assert(false);                                                                                                                                                                             \
        }                                                                                                                                                                                              \
    }

namespace Util
{
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

    inline std::unordered_set<std::string> filterExtensions(std::vector<std::string> availableExtensions, std::vector<std::string> requestedExtensions)
    {
        std::sort(availableExtensions.begin(), availableExtensions.end());
        std::sort(requestedExtensions.begin(), requestedExtensions.end());

        std::vector<std::string> result{};
        std::set_intersection(availableExtensions.begin(), availableExtensions.end(), requestedExtensions.begin(), requestedExtensions.end(), std::back_inserter(result));
        return std::unordered_set<std::string>(result.begin(), result.end());
    }
} // namespace Util

#endif // UTIL_H
