#include "GlfwContext.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/gl.h>

#include <stdexcept>

namespace pen
{

GlfwContext::GlfwContext()
{
    if(glfwInit() != GLFW_TRUE)
        throw std::runtime_error("Failed to initalize GLFW");
}

GlfwContext::~GlfwContext()
{
    glfwTerminate();
}

void GlfwContext::clear(const glm::vec3& clearColor) noexcept
{
    glClearColor(clearColor.r, clearColor.g, clearColor.b, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GlfwContext::pollEvents() noexcept
{
    glfwPollEvents();
}

void GlfwContext::swapBuffers() noexcept
{
    glfwSwapBuffers(glfwGetCurrentContext());
}

} // namespace pen
