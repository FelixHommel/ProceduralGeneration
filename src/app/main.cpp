#include "opengl/Shader.hpp"
#include "rendering/GlfwContext.hpp"
#include "rendering/Window.hpp"
#include "terrain/CubeLatticeScalarField3D.hpp"
#include "terrain/LatticeData3D.hpp"
#include "terrain/Sphere.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cmath>
#include <cstddef>
#include <exception>
#include <memory>
#include <print>
#include <utility>

namespace
{

constexpr std::size_t LATTICE_X{ 30 };
constexpr std::size_t LATTICE_Y{ 30 };
constexpr std::size_t LATTICE_Z{ 30 };

float computeDistanceABC(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c)
{
    const auto d{ (c - b) / glm::distance(c, b) };
    const auto v{ a - b };
    const auto t{ glm::dot(v, d) };
    const auto p{ b + (t * d) };

    return glm::distance(p, a);
}

void assignScalarField(
    pen::LatticeData3D<float, ::LATTICE_X, ::LATTICE_Y, ::LATTICE_Z>& scalarField, const glm::vec3& center
)
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
                scalarField[{ i, j, k }]
                    = static_cast<float>(std::max(std::exp(-0.0085f * distSq) - std::exp(-0.3f * perpDist), 0.f));
                // NOLINTEND(readability-magic-numbers)
            }
        }
    }
}

unsigned int vao{ 0 };                   // NOLINT
std::size_t numberOfVerticesToDraw{ 0 }; // NOLINT

void bufferGridDataGL(double isoLevel, pen::CubeLatticeScalarField3D<::LATTICE_X, ::LATTICE_Y, ::LATTICE_Z>& gridData)
{
    const auto vertices{ gridData.computeVertexDrawData(isoLevel) };
    numberOfVerticesToDraw = vertices.size() / 6;

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

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, nullptr);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, reinterpret_cast<void*>(sizeof(float) * 3));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void drawGrid()
{
    if(numberOfVerticesToDraw > 0)
    {
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(numberOfVerticesToDraw));
    }
}

} // namespace

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

    LatticeData3D<float, ::LATTICE_X, ::LATTICE_Y, ::LATTICE_Z> scalarField{};
    constexpr auto center{ glm::vec3(0.f) };
    ::assignScalarField(scalarField, center);

    constexpr auto gridSpacing{ 1.f };
    CubeLatticeScalarField3D<::LATTICE_X, ::LATTICE_Y, ::LATTICE_Z> grid{ gridSpacing, center, scalarField };

    constexpr auto isoLevel{ 0.5 };
    ::bufferGridDataGL(isoLevel, grid);

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
        ::drawGrid();

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

    glDeleteVertexArrays(1, &vao);

    return 0;
}
