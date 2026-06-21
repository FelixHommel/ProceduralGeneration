#include "GlfwContext.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

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

void GlfwContext::pollEvents() noexcept
{
    glfwPollEvents();
}

} // namespace pen
