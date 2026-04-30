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

void VkInitN::semaphoreSubmitInfo(VkSemaphoreSubmitInfo* info, VkPipelineStageFlags2 stageMask, VkSemaphore semaphore)
{
    info->sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    info->pNext = nullptr;
    info->semaphore = semaphore;
    info->stageMask = stageMask;
    info->deviceIndex = 0;
    info->value = 1;
}

void VkInitN::commandBufferSubmitInfo(VkCommandBufferSubmitInfo* info, VkCommandBuffer cmd)
{
    info->sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
    info->pNext = nullptr;
    info->commandBuffer = cmd;
    info->deviceMask = 0;
}

void VkInitN::submitInfo(VkSubmitInfo2* info, VkCommandBufferSubmitInfo* cmd, VkSemaphoreSubmitInfo* signalSemaphoreInfo, VkSemaphoreSubmitInfo* waitSemaphoreInfo)
{
    info->sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
    info->pNext = nullptr;

    info->waitSemaphoreInfoCount = waitSemaphoreInfo == nullptr ? 0 : 1;
    info->pWaitSemaphoreInfos = waitSemaphoreInfo;

    info->signalSemaphoreInfoCount = signalSemaphoreInfo == nullptr ? 0 : 1;
    info->pSignalSemaphoreInfos = signalSemaphoreInfo;

    info->commandBufferInfoCount = 1;
    info->pCommandBufferInfos = cmd;
}

VkImageSubresourceRange VkInitN::imageSubresourceRange(VkImageAspectFlags aspectMask)
{
    VkImageSubresourceRange subImage{};
    subImage.aspectMask = aspectMask;
    subImage.baseMipLevel = 0;
    subImage.levelCount = VK_REMAINING_MIP_LEVELS;
    subImage.baseArrayLayer = 0;
    subImage.layerCount = VK_REMAINING_ARRAY_LAYERS;

    return subImage;
}
