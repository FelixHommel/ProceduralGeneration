#include "opengl/Shader.hpp"
#include "rendering/GlfwContext.hpp"
#include "rendering/Window.hpp"
#include "terrain/Sphere.hpp"

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

    const auto marchingCubes{ Shader::loadFromFile(
        PEN_ROOT "resources/shaders/MarchingCubes.vert", PEN_ROOT "resources/shaders/MarchingCubes.frag"
    ) };
    const auto lighting{ Shader::loadFromFile(
        PEN_ROOT "resources/shaders/LightSource.vert", PEN_ROOT "resources/shaders/LightSource.frag"
    ) };

    const auto lightSphere{ std::make_unique<Sphere>() };
    lightSphere->copyToGPU();

    while(!window->shouldClose())
    {
        GlfwContext::pollEvents();
    }

    return 0;
}
