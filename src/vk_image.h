// Created by Jens Kromdijk 30/04/2026
//
#ifndef VK_IMAGE_H
#define VK_IMAGE_H

#include <vulkan/vulkan.h>

namespace VkImageN
{
    void transitionImage(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout);
}

#endif // VK_IMAGE_H
