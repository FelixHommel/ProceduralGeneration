#include "rendering/GlfwContext.hpp"
#include "rendering/Window.hpp"

#include <exception>
#include <memory>
#include <print>
#include <utility>

int main()
{
    using namespace pen;

    std::unique_ptr<GlfwContext> glfw;
    std::unique_ptr<Window> window;

    try
    {
        glfw = std::move(std::make_unique<GlfwContext>());
        window = std::move(std::make_unique<Window>("ProceduralGeneration"));
    }
    catch(const std::exception& e)
    {
        std::println(stderr, "Failure while setting up GLFW:\n{}", e.what());
        return 0;
    }

    while(!window->shouldClose())
    {
        GlfwContext::pollEvents();
    }

    return 0;
}
