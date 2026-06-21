#ifndef PEN_SRC_ENGINE_OPENGL_SHADER_HPP
#define PEN_SRC_ENGINE_OPENGL_SHADER_HPP

#include <glm/glm.hpp>

#include <filesystem>
#include <memory>

namespace pen
{

/// \brief RAII wrapper for an OpenGL shader program.
///
/// \author Felix Hommel
/// \date 6/21/2026
class Shader
{
public:
    /// \brief Create a new shader program
    ///
    /// \param pVertSource The source code of the vertex shader
    /// \param pFragSource The source code of the fragment shader
    Shader(const char* pVertSource, const char* pFragSource);
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    /// \brief Create a new \ref Shader from file.
    ///
    /// \param vert The path to the vertex shader source code
    /// \param frag The path to the fragment shader source code
    ///
    /// \returns \ref std::unique_ptr to a \ref Shader from \p vert and \p frag
    static std::unique_ptr<Shader> loadFromFile(const std::filesystem::path& vert, const std::filesystem::path& frag);

    /// \brief Get the native handle of the shader program.
    ///
    /// \returns The ID of the shader program
    [[nodiscard]] unsigned int handle() const noexcept { return m_id; }

    /// \brief Activate the shader program.
    void use() const noexcept;

    /// \brief Set a 3-component float vector uniform value.
    ///
    /// \param pName The name of the uniform
    /// \param vec The vector to upload to the uniform
    /// \param useShader (optional) Activate the shader before setting the uniform value
    void setVector3f(const char* pName, const glm::vec3& vec, bool useShader = false) const;

    /// \brief Set a 4x4 float matrix uniform value.
    ///
    /// \param pName The name of the uniform
    /// \param matrix The matrix to upload to the uniform
    /// \param useShader (optional) Activate the shader before setting the uniform value
    void setMatrix4f(const char* pName, const glm::mat4& matrix, bool useShader = false) const;

private:
    unsigned int m_id{ 0 };

    void release() const noexcept;
    int getUnifromLocation(const char* pName) const noexcept;
};

} // namespace pen

#endif // !PEN_SRC_ENGINE_OPENGL_SHADER_HPP
