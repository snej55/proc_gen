// Created by Jens Kromdijk 24/04/2026

#include "engine.h"
#include "util.h"
#include "vk_init.h"

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
}
