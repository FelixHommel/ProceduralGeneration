#include "Shader.hpp"

#include "utility/Assert.hpp"

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <spdlog/spdlog.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

namespace
{

constexpr std::size_t ERROR_LOG_SIZE{ 1024 }; ///< Size of the GL error log

/// \brief What kind of Shader is compiled.
///
/// \author Felix Hommel
/// \date 6/21/2026
enum class CompilationType : std::uint8_t
{
    Vertex,
    Fragment,
    Program
};

/// \brief Convert a \ref CompilationType to \ref std::string
///
/// \param type The \ref CompilationType that is being converted
///
/// \returns String representation of \p type
std::string compilationTypeToString(CompilationType type) noexcept
{
    switch(type)
    {
        using enum CompilationType;
    case Vertex:
        return "Vertex";
    case Fragment:
        return "Fragment";
    case Program:
        return "Program";
    default:
        return "<unknown>";
    }
}

/// \brief Convert a \ref CompilationType to \ref GLenum
///
/// \param type The \ref CompilationType that is being converted
///
/// \returns \ref GLenum equivalent of \p type
GLenum compilationTypeToGLenum(CompilationType type)
{
    switch(type)
    {
        using enum CompilationType;
    case Vertex:
        return GL_VERTEX_SHADER;
    case Fragment:
        return GL_FRAGMENT_SHADER;
    case Program:
        return GL_PROGRAM;
    default:
        throw std::runtime_error("Unknown compilation type");
    }
}

/// \brief Check a shader compilation or program linking process for errors.
///
/// \param id The id of the shader or program
/// \param type The \ref CompilationType of the OpenGL object that was processed
void checkShaderErrors(GLuint id, CompilationType type)
{
    int success{ 0 };
    std::array<char, ::ERROR_LOG_SIZE> infoLog{};

    if(type != CompilationType::Program)
    {
        glGetShaderiv(id, GL_COMPILE_STATUS, &success);
        if(success == 0)
        {
            glGetShaderInfoLog(id, static_cast<GLsizei>(infoLog.size()), nullptr, infoLog.data());
            spdlog::error("Shader compile-error({}):\n{}", ::compilationTypeToString(type), infoLog.data());
        }
    }
    else
    {
        glGetProgramiv(id, GL_LINK_STATUS, &success);
        if(success == 0)
        {
            glGetProgramInfoLog(id, static_cast<GLsizei>(infoLog.size()), nullptr, infoLog.data());
            spdlog::error("Program link-error({}):\n{}", ::compilationTypeToString(type), infoLog.data());
        }
    }
}

/// \brief Compile an OpenGL shader object.
///
/// \param pSource The source code of the shader
/// \param type The type of shader
///
/// \returns The ID of the shader
GLuint compileShader(const char* pSource, CompilationType type)
{
    PEN_ASSERT(type != CompilationType::Program, "Program is not a compilable type");

    if(pSource == nullptr)
    {
        spdlog::warn("No source was provided for {} shader", ::compilationTypeToString(type));

        return 0u;
    }

    const auto id{ glCreateShader(::compilationTypeToGLenum(type)) };
    glShaderSource(id, 1, &pSource, nullptr);
    glCompileShader(id);
    ::checkShaderErrors(id, type);

    return id;
}

/// \brief Read a file from the disk.
///
/// \param filepath The path to the file that is being read
///
/// \returns The contents of the file as \ref std::string as \ref std::optional, empty optional if the file can't be read
std::optional<std::string> readFile(const std::filesystem::path& filepath)
{
    if(!std::filesystem::exists(filepath))
        return std::nullopt;

    std::ifstream file(filepath);
    if(!file)
        return std::nullopt;

    std::stringstream buffer;
    buffer << file.rdbuf();

    return buffer.str();
}

} // namespace

namespace pen
{

Shader::Shader(const char* pVertSource, const char* pFragSource) : m_id{ glCreateProgram() }
{
    const auto vertId{ ::compileShader(pVertSource, ::CompilationType::Vertex) };
    const auto fragId{ ::compileShader(pFragSource, ::CompilationType::Fragment) };

    glAttachShader(m_id, vertId);
    glAttachShader(m_id, fragId);

    glLinkProgram(m_id);
    ::checkShaderErrors(m_id, ::CompilationType::Program);

    glDeleteShader(vertId);
    glDeleteShader(fragId);
}

Shader::~Shader()
{
    release();
}

Shader::Shader(Shader&& other) noexcept : m_id{ std::exchange(other.m_id, 0u) } {}

Shader& Shader::operator=(Shader&& other) noexcept
{
    if(this == &other)
        return *this;

    release();

    m_id = std::exchange(other.m_id, 0u);

    return *this;
}

std::unique_ptr<Shader> Shader::loadFromFile(const std::filesystem::path& vert, const std::filesystem::path& frag)
{
    auto result{ ::readFile(vert) };

    std::string vertSource;
    if(result.has_value())
        vertSource = std::move(result.value());

    result = ::readFile(frag);

    std::string fragSource;
    if(result.has_value())
        fragSource = std::move(result.value());

    return std::make_unique<Shader>(vertSource.c_str(), fragSource.c_str());
}

void Shader::use() const noexcept
{
    glUseProgram(m_id);
}

void Shader::setVector3f(const char* pName, const glm::vec3& vec, bool useShader) const
{
    if(useShader)
        use();

    glUniform3fv(getUnifromLocation(pName), 1, glm::value_ptr(vec));
}

void Shader::setMatrix4f(const char* pName, const glm::mat4& matrix, bool useShader) const
{
    if(useShader)
        use();

    glUniformMatrix4fv(getUnifromLocation(pName), 1, GL_FALSE, glm::value_ptr(matrix));
}

/// \brief Delete the shader program.
void Shader::release() const noexcept
{
    if(m_id != 0u)
        glDeleteProgram(m_id);
}

/// \brief Find the location of a uniform
///
/// \param pName The name of the uniform
///
/// \returns The location of the \p pName uniform
int Shader::getUnifromLocation(const char* pName) const noexcept
{
    return glGetUniformLocation(m_id, pName);
}

} // namespace pen
