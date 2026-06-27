#include "Application.hpp"

#include "opengl/Shader.hpp"
#include "rendering/GlfwContext.hpp"
#include "rendering/Window.hpp"
#include "terrain/Sphere.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/gl.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <memory>

namespace
{

constexpr auto center{ glm::vec3(0.f) };
constexpr auto gridSpacing{ 1.f };
constexpr auto isoLevel{ 0.5 };

constexpr auto lightColor{ glm::vec3(1.f) };
constexpr auto objectColor{ glm::vec3(0.99609375f, 0.80078125f, 0.31640625f) };

constexpr auto cameraRotateRadius{ 60.f };
constexpr auto lightPosRadius{ 25.f };
constexpr auto cameraY{ 8.f };

constexpr auto cameraRotationMod{ 1.5f };
constexpr auto lightRotationMod{ 5.f };

constexpr auto movingScene{ false };

float computeDistanceABC(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c)
{
    const auto d{ (c - b) / glm::distance(c, b) };
    const auto v{ a - b };
    const auto t{ glm::dot(v, d) };
    const auto p{ b + (t * d) };

    return glm::distance(p, a);
}

} // namespace

namespace pen::app
{

Application::Application()
    : glfw{ std::make_unique<GlfwContext>() }
    , window{ std::make_unique<Window>("ProceduralGeneration") }
    , marchingCubes{ Shader::loadFromFile(
          PEN_ROOT "resources/shaders/MarchingCubes.vert", PEN_ROOT "resources/shaders/MarchingCubes.frag"
      ) }
    , lighting{ Shader::loadFromFile(
          PEN_ROOT "resources/shaders/LightSource.vert", PEN_ROOT "resources/shaders/LightSource.frag"
      ) }
    , lightSphere{ std::make_unique<Sphere>() }
{
    scalarField = std::make_unique<LatticeData>();
    grid = std::make_unique<ScalarField>(gridSpacing, center, *scalarField);

    lightSphere->copyToGPU();

    assignScalarField(center);
    bufferGridDataGL(isoLevel);
}

Application::~Application()
{
    glDeleteVertexArrays(1, &vao);
}

void Application::renderLoop()
{
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

        update();
        render();

        GlfwContext::swapBuffers();
        GlfwContext::pollEvents();
    }
}

void Application::update() {}
void Application::render()
{
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

    marchingCubes->setVector3f("lightPos", lightPos, true);
    marchingCubes->setVector3f("lightColor", lightColor);
    marchingCubes->setVector3f("objectColor", objectColor);
    marchingCubes->setMatrix4f("model", marchingCubesModel);
    marchingCubes->setMatrix4f("view", view);
    marchingCubes->setVector3f("viewPos", viewPos);

    drawGrid();

    const auto lightingModel{ glm::translate(glm::mat4(1.f), lightPos) };

    lighting->setVector3f("lightColor", lightColor, true);
    lighting->setMatrix4f("model", lightingModel);
    lighting->setMatrix4f("view", view);

    lightSphere->draw();
}

void Application::assignScalarField(const glm::vec3& center)
{
    for(std::size_t i{ 0 }; i < ::LATTICE_X; ++i)
    {
        for(std::size_t j{ 0 }; j < ::LATTICE_Y; ++j)
        {
            for(std::size_t k{ 0 }; k < ::LATTICE_Z; ++k)
            {
                const auto distance{ glm::vec3(
                    static_cast<float>(i) - center[0] - (static_cast<float>(::LATTICE_X) / 2.f),
                    static_cast<float>(j) - center[1] - (static_cast<float>(::LATTICE_Y) / 2.f),
                    static_cast<float>(k) - center[2] - (static_cast<float>(::LATTICE_Z) / 2.f)
                ) };

                const auto distSq{ glm::length(distance) };
                const auto perpDist{ ::computeDistanceABC(distance, glm::vec3(0.f), { 1.f, 0.f, 0.f }) };

                // NOLINTBEGIN(readability-magic-numbers)
                (*scalarField)[{ i, j, k }]
                    = static_cast<float>(std::max(std::exp(-0.0085f * distSq) - std::exp(-0.3f * perpDist), 0.f));
                // NOLINTEND(readability-magic-numbers)
            }
        }
    }
}

void Application::bufferGridDataGL(double isoLevel)
{
    constexpr auto VETEX_ATTRIBUTE_COUNT{ 6 };

    const auto vertices{ grid->computeVertexDrawData(isoLevel) };
    spdlog::info("vertices: {}", vertices.size());
    numberOfVerticesToDraw = vertices.size() / VETEX_ATTRIBUTE_COUNT;

    if(numberOfVerticesToDraw == 0)
        return;

    unsigned int vbo{ 0 };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(
        GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(float) * vertices.size()), vertices.data(), GL_STATIC_DRAW
    );

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * VETEX_ATTRIBUTE_COUNT, nullptr);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(
        1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * VETEX_ATTRIBUTE_COUNT, reinterpret_cast<void*>(sizeof(float) * 3)
    );
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Application::drawGrid() const
{
    if(numberOfVerticesToDraw > 0)
    {
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(numberOfVerticesToDraw));
    }
}

} // namespace pen::app
