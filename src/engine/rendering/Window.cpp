#include "Window.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/gl.h>

#include <stdexcept>
#include <string>
#include <utility>

namespace pen
{

Window::Window(const std::string& title, int width, int height) : m_viewport{ .width = width, .height = height }
{
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, Window::CONTEXT_VERSION_MAJ);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, Window::CONTEXT_VERSION_MIN);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if defined(__APPLE__)
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    if(m_viewport.width <= 0 || m_viewport.height <= 0)
    {
        const auto* vm{ glfwGetVideoMode(glfwGetPrimaryMonitor()) };

        m_viewport = { .width = vm->width, .height = vm->height };
    }

    m_window = glfwCreateWindow(m_viewport.width, m_viewport.height, title.c_str(), nullptr, nullptr);

    if(m_window == nullptr)
    {
        throw std::runtime_error("Failed to create a GLFW window");
    }

    glfwMakeContextCurrent(m_window);
    glfwSetWindowUserPointer(m_window, this);

    // NOTE: Maybe want to extract into separate GLAD context later.
    if(gladLoadGL(glfwGetProcAddress) == 0)
    {
        destroyWindow();

        throw std::runtime_error("GLAD failed to load OpenGL");
    }

    glViewport(0, 0, m_viewport.width, m_viewport.height);

    glfwSetWindowSizeCallback(m_window, [](GLFWwindow* window, int width, int height) {
        auto* self{ static_cast<Window*>(glfwGetWindowUserPointer(window)) };

        self->m_viewport = { .width = width, .height = height };
        glViewport(0, 0, self->m_viewport.width, self->m_viewport.height);
    });
}

Window::~Window()
{
    destroyWindow();
}

Window::Window(Window&& other) noexcept
    : m_window{ std::exchange(other.m_window, nullptr) }, m_viewport{ std::exchange(other.m_viewport, {}) }
{}

Window& Window::operator=(Window&& other) noexcept
{
    if(this == &other)
        return *this;

    destroyWindow();

    m_window = std::exchange(other.m_window, nullptr);
    m_viewport = std::exchange(other.m_viewport, {});

    return *this;
}

/// \brief Destroy the window if it is valid.
void Window::destroyWindow()
{
    if(m_window == nullptr)
        return;

    glfwDestroyWindow(m_window);
    m_window = nullptr;
}

} // namespace pen
