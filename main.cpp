// Created by Jens Kromdijk 05/04/2026

#include <fmt/base.h>

#include <cstdlib>

#define _DEBUG
#include "src/engine.hpp"

int main()
{
    Engine engine{};

    try
    {
        engine.init();
    }
    catch (std::exception& e)
    {
        fmt::println("{}", e.what());
        return EXIT_FAILURE;
    }

    fmt::println("We ran!");

    return EXIT_SUCCESS;
}
