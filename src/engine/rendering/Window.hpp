#ifndef PEN_SRC_ENGINE_RENDERING_WINDOW_HPP
#define PEN_SRC_ENGINE_RENDERING_WINDOW_HPP

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <string>

namespace pen
{

/// \brief Dimensions of the Window viewport
///
/// \author Felix Hommel
/// \date 6/20/2026
struct Viewport
{
    int width{ 0 };
    int height{ 0 };
};

/// \brief RAII wrapper of a \ref GLFWwindow
///
/// Takes care of creation and destruction of a \ref GLFWwindow and loads OpenGL functions using GLAD.
///
/// \author Felix Hommel
/// \date 6/21/2026
///
/// \note Assumes that a GLFW session was initialized prior to creating a \ref Window
class Window
{
public:
    /// \brief Create a new \ref Window
    ///
    /// If either \p width or \p height are 0 or less, the window will be created in fullscreen mode.
    ///
    /// \param title The title of the window
    /// \param width (optional) The width of the window
    /// \param height (optional) The height of the window
    explicit Window(const std::string& title, int width = 0, int height = 0);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&& other) noexcept;
    Window& operator=(Window&& other) noexcept;

    [[nodiscard]] bool shouldClose() const noexcept { return glfwWindowShouldClose(m_window) == GLFW_TRUE; }

private:
    static constexpr auto CONTEXT_VERSION_MAJ{ 3 };
    static constexpr auto CONTEXT_VERSION_MIN{ 3 };

    GLFWwindow* m_window{ nullptr };
    Viewport m_viewport;

    void destroyWindow();
};

} // namespace pen

#endif // !PEN_SRC_ENGINE_RENDERING_WINDOW_HPP
