// Created by Jens Kromdijk 28/04/2026

#include "vk_init.h"

void VkInitN::commandPoolCreateInfo(VkCommandPoolCreateInfo* commandCI, uint32_t queueFamilyIndex)
{
    commandCI->sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandCI->pNext = nullptr;
    commandCI->flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    commandCI->queueFamilyIndex = queueFamilyIndex;
}

void VkInitN::commandBufferAllocInfo(VkCommandBufferAllocateInfo* allocInfo, VkCommandPool commandPool, uint32_t count)
{
    allocInfo->sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo->pNext = nullptr;
    allocInfo->commandPool = commandPool;
    allocInfo->commandBufferCount = count;
    allocInfo->level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
}

void VkInitN::fenceCreateInfo(VkFenceCreateInfo* fenceCI, VkFenceCreateFlags flags)
{
    fenceCI->sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCI->pNext = nullptr;
    fenceCI->flags = flags;
}

void VkInitN::semaphoreCreateInfo(VkSemaphoreCreateInfo* semaphoreCI, VkSemaphoreCreateFlags flags)
{
    semaphoreCI->sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
    semaphoreCI->pNext = nullptr;
    semaphoreCI->flags = flags;
}

void VkInitN::commandBufferBeginInfo(VkCommandBufferBeginInfo* cmdBufferCI, VkCommandBufferUsageFlags flags)
{
    cmdBufferCI->sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBufferCI->pNext = nullptr;

    cmdBufferCI->pInheritanceInfo = nullptr;
    cmdBufferCI->flags = flags;
}
