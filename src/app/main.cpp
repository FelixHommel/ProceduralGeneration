#include "opengl/Shader.hpp"
#include "rendering/GlfwContext.hpp"
#include "rendering/Window.hpp"
#include "terrain/Sphere.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cmath>
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

    constexpr auto lightColor{ glm::vec3(1.f) };
    constexpr auto objectColor{ glm::vec3(0.99609375f, 0.80078125f, 0.31640625f) };

    constexpr auto cameraRotateRadius{ 60.f };
    constexpr auto lightPosRadius{ 25.f };
    constexpr auto cameraY{ 8.f };

    constexpr auto cameraRotationMod{ 1.5f };
    constexpr auto lightRotationMod{ 5.f };

    constexpr auto movingScene{ false };

    const auto windowSize{ window->viewport() };
    const auto projection{ glm::perspective(
        glm::radians(45.f), static_cast<float>(windowSize.width) / static_cast<float>(windowSize.height), 0.1f, 100.f
    ) };

    marchingCubes->use();
    marchingCubes->setMatrix4f("projection", projection);

    lighting->use();
    lighting->setMatrix4f("projection", projection);

    while(!window->shouldClose())
    {
        GlfwContext::clear();

        auto viewPos{ glm::vec3(0.f) };
        auto lightPos{ glm::vec3(0.f) };

        if constexpr(movingScene)
        {
            const auto lightX{ static_cast<float>(std::sin(lightRotationMod * glfwGetTime()) * lightPosRadius) };
            const auto lightZ{ static_cast<float>(std::cos(lightRotationMod * glfwGetTime()) * lightPosRadius) };
            const auto cameraX{ static_cast<float>(std::sin(cameraRotationMod * glfwGetTime()) * cameraRotateRadius) };
            const auto cameraZ{ static_cast<float>(std::cos(cameraRotationMod * glfwGetTime()) * cameraRotateRadius) };

            viewPos = glm::vec3(cameraX, cameraY, cameraZ);
            lightPos = glm::vec3(lightX, 0.f, lightZ);
        }
        else
        {
            viewPos = glm::vec3(-1.f, cameraY, -1.f);
            lightPos = glm::vec3(0.f);
        }

        auto view{ glm::mat4(1.f) };
        view = glm::lookAt(viewPos, glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f));

        const auto marchingCubesModel{ glm::mat4(1.f) };

        marchingCubes->use();
        marchingCubes->setVector3f("lightPos", lightPos);
        marchingCubes->setVector3f("lightColor", lightColor);
        marchingCubes->setVector3f("objectColor", objectColor);
        marchingCubes->setVector3f("viewPos", viewPos);

        marchingCubes->setMatrix4f("model", marchingCubesModel);
        marchingCubes->setMatrix4f("view", view);

        // TODO: Draw marching cubes here

        auto lightingModel{ glm::mat4(1.f) };
        lightingModel = glm::translate(lightingModel, lightPos);

        lighting->use();
        lighting->setVector3f("lightColor", lightColor);

        lighting->setMatrix4f("model", lightingModel);
        lighting->setMatrix4f("view", view);

        lightSphere->draw();

        GlfwContext::swapBuffers();
        GlfwContext::pollEvents();
    }

    return 0;
}
