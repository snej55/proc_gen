// Created by Jens Kromdijk 28/04/2026

#ifndef VK_INIT_H
#define VK_INIT_H

#include <vulkan/vulkan.h>

namespace VkInitN
{
    void commandPoolCreateInfo(VkCommandPoolCreateInfo* commandCI, uint32_t queueFamilyIndex);

    void commandBufferAllocInfo(VkCommandBufferAllocateInfo* allocInfo, VkCommandPool commandPool, uint32_t count);

    void fenceCreateInfo(VkFenceCreateInfo* fenceCI, VkFenceCreateFlags flags);

    void semaphoreCreateInfo(VkSemaphoreCreateInfo* semaphoreCI, VkSemaphoreCreateFlags flags);

    void commandBufferBeginInfo(VkCommandBufferBeginInfo* cmdBufferCI, VkCommandBufferUsageFlags flags);
}; // namespace VkInitN

#endif // VK_INIT_H
