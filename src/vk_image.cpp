// Created by Jens Kromdijk 30/04/2026

#include "vk_image.h"
#include "vk_init.h"

#include <volk/volk.h> // vkCmdPipelineBarrier2 is defined here for some reason

void VkImageN::transitionImage(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout)
{
    VkImageMemoryBarrier2 imageBarrier{.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2};
    imageBarrier.pNext = nullptr;

    // TODO: Improve flags:
    // - VK_PIPELINE_STAGE_2_NONE (coming from undefined) for layout transitions
    // - VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT for compute -> graphics
    imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    imageBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
    // - VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT for layout transition
    // - VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT for compute -> graphics
    imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    imageBarrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;

    imageBarrier.oldLayout = currentLayout;
    imageBarrier.newLayout = newLayout;

    VkImageAspectFlags aspectMask{(newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT};
    imageBarrier.subresourceRange = VkInitN::imageSubresourceRange(aspectMask);
    imageBarrier.image = image;

    VkDependencyInfo depInfo{.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO};
    depInfo.pNext = nullptr;

    depInfo.imageMemoryBarrierCount = 1;
    depInfo.pImageMemoryBarriers = &imageBarrier;

    vkCmdPipelineBarrier2(cmd, &depInfo);
}
