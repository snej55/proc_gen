// Created by Jens Kromdijk 24/04/2026

#include "engine.h"
#include "util.h"
#include "vk_init.h"
#include "vk_image.h"

#include <cassert>
#include <chrono>
#include <thread>

void Engine::init()
{
    assert(!m_init);
    m_context = std::make_unique<Context>();
    m_context->init();
}

void Engine::run()
{
    SDL_Event e;
    bool quit{false};
    bool minimized{false};
    while (!quit)
    {
        while (SDL_PollEvent(&e) != 0)
        {
            if (e.type == SDL_EVENT_QUIT)
            {
                quit = true;
            }

            if (e.type == SDL_EVENT_WINDOW_MINIMIZED)
            {
                minimized = true;
            }
            if (e.type == SDL_EVENT_WINDOW_RESTORED)
            {
                minimized = false;
            }
        }

        if (minimized)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        draw();
    }
}

void Engine::free() { m_context->free(); }

void Engine::draw()
{
    // make sure everything is finished
    // timeout is in nanoseconds
    VK_CHECK(vkWaitForFences(m_context->getDevice(), 1, &m_context->getCurrentFrame().m_renderFence, true, 1e+9));
    VK_CHECK(vkResetFences(m_context->getDevice(), 1, &m_context->getCurrentFrame().m_renderFence));

    uint32_t swapchainImageIndex;
    VK_CHECK(vkAcquireNextImageKHR(m_context->getDevice(), m_context->getSwapchain(), 1e+9, m_context->getCurrentFrame().m_swapchainSemaphore, nullptr, &swapchainImageIndex));

    VkCommandBuffer cmd{m_context->getCurrentFrame().m_commandBuffer};
    // reset the command buffer
    VK_CHECK(vkResetCommandBuffer(cmd, 0));

    // begin recording
    VkCommandBufferBeginInfo cmdBeginInfo{};
    VkInitN::commandBufferBeginInfo(&cmdBeginInfo, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    // start the actual recording :)
    VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

    VkImageN::transitionImage(cmd, m_context->getSwapchainImages()[swapchainImageIndex], VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL);

    VkClearColorValue clearValue;
    const float flash{std::abs(std::sin(static_cast<float>(m_context->getFrameNumber() / 120.f)))};
    clearValue = {{0.0f, 0.0f, flash, 1.0f}};

    VkImageSubresourceRange clearRange{VkInitN::imageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT)};

    // clear image
    vkCmdClearColorImage(cmd, m_context->getSwapchainImages()[swapchainImageIndex], VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);
    // make it into presentable mode
    VkImageN::transitionImage(cmd, m_context->getSwapchainImages()[swapchainImageIndex], VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    // finish command buffer (can be executed now)
    VK_CHECK(vkEndCommandBuffer(cmd));

    // prepare queue submission
    VkCommandBufferSubmitInfo cmdInfo{};
    VkInitN::commandBufferSubmitInfo(&cmdInfo, cmd);

    VkSemaphoreSubmitInfo waitInfo{};
    VkInitN::semaphoreSubmitInfo(&waitInfo, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, m_context->getCurrentFrame().m_swapchainSemaphore);
    VkSemaphoreSubmitInfo signalInfo{};
    VkInitN::semaphoreSubmitInfo(&signalInfo, VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, m_context->getCurrentFrame().m_renderSemaphore);

    VkSubmitInfo2 submit;
    VkInitN::submitInfo(&submit, &cmdInfo, &signalInfo, &waitInfo);

    VK_CHECK(vkQueueSubmit2(m_context->getGraphicsQueue(), 1, &submit, m_context->getCurrentFrame().m_renderFence));

    // present the actual image
    VkPresentInfoKHR presentInfo{.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    presentInfo.pNext = nullptr;
    presentInfo.pSwapchains = &m_context->getSwapchain(); // returns swapchain by reference so should be fine
    presentInfo.swapchainCount = 1;
    presentInfo.pWaitSemaphores = &m_context->getCurrentFrame().m_renderSemaphore;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pImageIndices = &swapchainImageIndex;

    VK_CHECK(vkQueuePresentKHR(m_context->getGraphicsQueue(), &presentInfo));
    m_context->tickFrame();
}
