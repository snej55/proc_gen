// Created by Jens Kromdijk 24/04/2026

#ifndef ENGINE_H
#define ENGINE_H

#include "context.hpp"

#include <memory>

class Engine
{
public:
    Engine() = default;
    ~Engine() = default;

    void init();

    void update();

    [[nodiscard]] bool getInit() const { return m_init; }
    [[nodiscard]] Context* getContext() { return m_context.get(); }

private:
    bool m_init{false};
    std::unique_ptr<Context> m_context{nullptr};
};

#endif
