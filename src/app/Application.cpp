#include "Application.hpp"

#include "opengl/Shader.hpp"
#include "rendering/Camera.hpp"
#include "rendering/GlfwContext.hpp"
#include "rendering/Window.hpp"
#include "terrain/Sphere.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/gl.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <memory>

namespace
{

constexpr auto CENTER{ glm::vec3(0.f) };
constexpr auto GRID_SPACING{ 1.f };
constexpr auto ISO_LEVEL{ 0.5 };

constexpr auto LIGHT_COLOR{ glm::vec3(1.f) };
constexpr auto OBJECT_COLOR{ glm::vec3(0.99609375f, 0.80078125f, 0.31640625f) };

} // namespace

namespace pen::app
{

Application::Application()
    : m_glfw{ std::make_unique<GlfwContext>() }
    , m_window{ std::make_unique<Window>("ProceduralGeneration") }
    , m_marchingCubesShader{ Shader::loadFromFile(
          PEN_ROOT "resources/shaders/MarchingCubes.vert", PEN_ROOT "resources/shaders/MarchingCubes.frag"
      ) }
    , m_lightingShader{ Shader::loadFromFile(
          PEN_ROOT "resources/shaders/LightSource.vert", PEN_ROOT "resources/shaders/LightSource.frag"
      ) }
    , m_lightSphere{ std::make_unique<Sphere>() }
    , m_scalarField{ std::make_unique<LatticeData>() }
    , m_camera{ std::make_unique<Camera>() }
{
    m_window->setWindowUserPointer(this);
    m_window->registerWindowResizeCallback([](GLFWwindow* window, int width, int height) {
        auto* self{ static_cast<Application*>(glfwGetWindowUserPointer(window)) };
        self->m_window->setViewport(width, height);
    });
    m_window->registerCursorPosCallback([](GLFWwindow* window, double posXIn, double posYIn) {
        auto* self{ static_cast<Application*>(glfwGetWindowUserPointer(window)) };

        const auto posX{ static_cast<float>(posXIn) };
        const auto posY{ static_cast<float>(posYIn) };

        if(self->m_firstMouse)
        {
            self->m_lastMouseX = posX;
            self->m_lastMouseY = posY;
            self->m_firstMouse = false;
        }

        const auto offsetX{ posX - self->m_lastMouseX };
        const auto offsetY{ posY - self->m_lastMouseY };

        self->m_lastMouseX = posX;
        self->m_lastMouseY = posY;

        self->m_camera->processMouseMovement(offsetX, offsetY);
    });
    m_window->registerScrollCallback([](GLFWwindow* window, double, double offsetY) {
        auto* self{ static_cast<Application*>(glfwGetWindowUserPointer(window)) };

        self->m_camera->porocessMouseScroll(static_cast<float>(offsetY));
    });

    m_lightSphere->copyToGPU();

    assignScalarField(CENTER);
    m_grid = std::make_unique<ScalarField>(GRID_SPACING, CENTER, *m_scalarField);

    bufferGridDataGL(ISO_LEVEL);
}

Application::~Application()
{
    glDeleteVertexArrays(1, &m_vao);
}

void Application::start()
{
    float deltaTime{ 0.f };
    float lastTime{ 0.f };
    while(!m_window->shouldClose())
    {
        glDisable(GL_CULL_FACE);

        const auto currentTime{ static_cast<float>(glfwGetTime()) };
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        GlfwContext::clear();

        update(deltaTime);
        render();

        GlfwContext::swapBuffers();
        GlfwContext::pollEvents();
    }
}

/// \brief Update the interactive systems.
void Application::update(float deltaTime) const
{
    processInput(m_window->handle(), deltaTime);
}

/// \brief Render the scene to the screen.
void Application::render() const
{
    const auto [w, h]{ m_window->viewport() };
    const auto projection{
        glm::perspective(glm::radians(m_camera->zoom()), static_cast<float>(w) / static_cast<float>(h), 0.1f, 100.f)
    };

    const auto view{ m_camera->viewMatrix() };
    const auto viewPos{ m_camera->position() };

    constexpr auto lightPos{ glm::vec3(0.f) };
    constexpr auto marchingCubesModel{ glm::mat4(1.f) };

    m_marchingCubesShader->setVector3f("lightPos", lightPos, true);
    m_marchingCubesShader->setVector3f("lightColor", LIGHT_COLOR);
    m_marchingCubesShader->setVector3f("objectColor", OBJECT_COLOR);
    m_marchingCubesShader->setMatrix4f("model", marchingCubesModel);
    m_marchingCubesShader->setMatrix4f("view", view);
    m_marchingCubesShader->setMatrix4f("projection", projection);
    m_marchingCubesShader->setVector3f("viewPos", viewPos);

    if(m_numberOfVerticesToDraw > 0)
    {
        glBindVertexArray(m_vao);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(m_numberOfVerticesToDraw));
    }

    constexpr auto lightingModel{ glm::translate(glm::mat4(1.f), lightPos) };

    m_lightingShader->setVector3f("lightColor", LIGHT_COLOR, true);
    m_lightingShader->setMatrix4f("model", lightingModel);
    m_lightingShader->setMatrix4f("view", view);
    m_lightingShader->setMatrix4f("projection", projection);

    m_lightSphere->draw();
}

/// \brief Check for user input and update the state of the app.
void Application::processInput(GLFWwindow* window, float dt) const
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        m_camera->processKeyboard(Camera::CameraMovement::FORWARD, dt);
    else if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        m_camera->processKeyboard(Camera::CameraMovement::BACKWARD, dt);

    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        m_camera->processKeyboard(Camera::CameraMovement::LEFT, dt);
    else if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        m_camera->processKeyboard(Camera::CameraMovement::RIGHT, dt);

    if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        m_camera->processKeyboard(Camera::CameraMovement::UP, dt);
    else if(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        m_camera->processKeyboard(Camera::CameraMovement::DOWN, dt);

    if(glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    if(glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        m_camera->processKeyboard(Camera::CameraMovement::PITCH_UP, dt);
    else if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        m_camera->processKeyboard(Camera::CameraMovement::PITCH_DOWN, dt);

    if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        m_camera->processKeyboard(Camera::CameraMovement::YAW_RIGHT, dt);
    else if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        m_camera->processKeyboard(Camera::CameraMovement::YAW_LEFT, dt);
}

/// \brief Initialize the scalar field
///
/// \param center (optional) Where the center of the field is
void Application::assignScalarField(const glm::vec3& center) const
{
    constexpr float R{ 8.f };
    constexpr float r{ 3.f };

    for(std::size_t i = 0; i < ::LATTICE_X; ++i)
    {
        for(std::size_t j = 0; j < ::LATTICE_Y; ++j)
        {
            for(std::size_t k = 0; k < ::LATTICE_Z; ++k)
            {
                const auto x{ static_cast<float>(i) - (static_cast<float>(::LATTICE_X) / 2.f) + center.x };
                const auto y{ static_cast<float>(j) - (static_cast<float>(::LATTICE_Y) / 2.f) + center.y };
                const auto z{ static_cast<float>(k) - (static_cast<float>(::LATTICE_Z) / 2.f) + center.z };

                const auto distToRing{
                    std::sqrt(((std::sqrt((x * x) + (z * z)) - R) * (std::sqrt((x * x) + (z * z)) - R)) + (y * y))
                };

                // NOLINTBEGIN(readability-magic-numbers)
                (*m_scalarField)[{ i, j, k }] = std::max(0.f, 1.f - (distToRing / r));
                // NOLINTEND(readability-magic-numbers)
            }
        }
    }
}

/// \brief Upload the vertex data to the GPU.
///
/// \param isoLevel how detailed to mesh is.
void Application::bufferGridDataGL(double isoLevel)
{
    constexpr auto VERTEX_ATTRIBUTE_COUNT{ 6 };

    const auto vertices{ m_grid->computeVertexDrawData(isoLevel) };
    m_numberOfVerticesToDraw = vertices.size() / VERTEX_ATTRIBUTE_COUNT;

    if(m_numberOfVerticesToDraw == 0)
        return;

    unsigned int vbo{ 0 };

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(
        GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(float) * vertices.size()), vertices.data(), GL_STATIC_DRAW
    );

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * VERTEX_ATTRIBUTE_COUNT, nullptr);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(
        1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * VERTEX_ATTRIBUTE_COUNT, reinterpret_cast<void*>(sizeof(float) * 3)
    );
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

} // namespace pen::app
