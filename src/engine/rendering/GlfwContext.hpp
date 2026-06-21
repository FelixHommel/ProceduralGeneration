#ifndef PEN_SRC_ENGINE_RENDERING_GLFW_CONTEXT_HPP
#define PEN_SRC_ENGINE_RENDERING_GLFW_CONTEXT_HPP

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

    /// \brief Provide GLFW with new events.
    static void pollEvents() noexcept;
};

} // namespace pen

#endif // !PEN_SRC_ENGINE_RENDERING_GLFW_CONTEXT_HPP
