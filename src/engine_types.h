// Created by Jens Kromdijk 26/04/2026

#ifndef ENGINE_TYPES_H
#define ENGINE_TYPES_H

#include <stdexcept>

#include <vulkan/vulkan.h>

struct EngineResult
{
    void* m_data;
    int m_code;
    VkResult m_vkResult;
    const char* m_msg;
};

class EngineException : public std::runtime_error
{
public:
    EngineResult m_result;

    EngineException(EngineResult result, const std::string& msg) : std::runtime_error(msg), m_result(result) {}
};

#endif // ENGINE_TYPES_H
