#ifndef PEN_SRC_ENGINE_RENDERING_GLFW_CONTEXT_HPP
#define PEN_SRC_ENGINE_RENDERING_GLFW_CONTEXT_HPP

#include <glm/glm.hpp>

namespace
{

namespace defaults
{

constexpr auto CLEAR_COLOR{ glm::vec3(0.f) };

} // namespace defaults

} // namespace

namespace pen
{

/// \brief Simple RAII wrapper for the GLFW session.
///
/// \author Felix Hommel
/// \date 6/21/2026
class GlfwContext
{
public:
    GlfwContext();
    ~GlfwContext();

    GlfwContext(const GlfwContext&) = delete;
    GlfwContext& operator=(const GlfwContext&) = delete;
    GlfwContext(GlfwContext&&) = delete;
    GlfwContext& operator=(GlfwContext&&) = delete;

    /// \brief Clear the framebuffer
    static void clear(const glm::vec3& clearColor = ::defaults::CLEAR_COLOR) noexcept;

    /// \brief Provide GLFW with new events.
    static void pollEvents() noexcept;

    /// \brief Swap framebuffers.
    static void swapBuffers() noexcept;
};

} // namespace pen

#endif // !PEN_SRC_ENGINE_RENDERING_GLFW_CONTEXT_HPP
