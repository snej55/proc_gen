// Created by Jens Kromdijk 24/04/2026

#include "engine.hpp"

#include <cassert>

void Engine::init()
{
    assert(!m_init);
    m_context = std::make_unique<Context>();
    m_context->init();
}
