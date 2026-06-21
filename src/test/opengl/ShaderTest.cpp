#include "opengl/Shader.hpp"
#include "testUtility/OpenGLTestFixture.hpp"
#include "testUtility/RandomNumberGenerator.hpp"

#include <gtest/gtest.h>

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <array>
#include <cstddef>
#include <memory>
#include <span>
#include <type_traits>
#include <utility>

namespace pen::testing
{

/// \brief Test the features of the \ref Shader class.
///
/// \author Felix Hommel
/// \date 1/24/2026
class ShaderTest : public ::testing::Test
{
public:
    ShaderTest() = default;
    ~ShaderTest() override = default;

    ShaderTest(const ShaderTest&) = delete;
    ShaderTest(ShaderTest&&) = delete;
    ShaderTest& operator=(const ShaderTest&) = delete;
    ShaderTest& operator=(ShaderTest&&) = delete;

    void SetUp() override
    {
        if(!m_glContext->setup())
            GTEST_SKIP() << m_glContext->getSkipReason();

        m_shader = std::make_unique<Shader>(VERTEX_SHADER_SRC, FRAGMENT_SHADER_SRC);

        m_float3Location = glGetUniformLocation(m_shader->handle(), FLOAT_3_UNIFORM);
        m_matrix4Location = glGetUniformLocation(m_shader->handle(), MATRIX_4_UNIFORM);
    }

    void TearDown() override { m_glContext->teardown(); }

protected:
    static constexpr auto VERTEX_SHADER_SRC{ R"(
        #version 330 core

        uniform vec3 float3Uniform;
        uniform mat4 mat4Uniform;

        void main()
        {
            vec4 result = mat4Uniform * vec4(float3Uniform, 1.0);
            gl_Position = normalize(result);
        }
    )" };
    static constexpr auto FRAGMENT_SHADER_SRC{ R"(
        #version 330 core
        out vec4 FragColor;

        void main() { FragColor = vec4(1.0); }
    )" };

    static constexpr auto FLOAT_3_UNIFORM{ "float3Uniform" };
    static constexpr auto MATRIX_4_UNIFORM{ "mat4Uniform" };

    std::unique_ptr<OpenGLTestFixture> m_glContext{ std::make_unique<OpenGLTestFixture>() };
    std::unique_ptr<Shader> m_shader;

    int m_float3Location{ 0 };
    int m_matrix4Location{ 0 };

    template<typename T>
        requires std::is_floating_point_v<T> || std::is_integral_v<T>
    void getUniformValue(int location, T& out)
    {
        if constexpr(std::is_same_v<T, GLfloat>)
            glGetUniformfv(m_shader->handle(), location, &out);
        else if constexpr(std::is_same_v<T, GLint>)
            glGetUniformiv(m_shader->handle(), location, &out);
        else if constexpr(std::is_same_v<T, GLuint>)
            glGetUniformuiv(m_shader->handle(), location, &out);
        else
            static_assert(!sizeof(T), "Unsupported uniform element type for glGetUniform");
    }

    template<typename T>
    void getUniformValue(int location, std::span<T> out)
    {
        if constexpr(std::is_same_v<T, GLfloat>)
            glGetUniformfv(m_shader->handle(), location, out.data());
        else if constexpr(std::is_same_v<T, GLint>)
            glGetUniformiv(m_shader->handle(), location, out.data());
        else if constexpr(std::is_same_v<T, GLuint>)
            glGetUniformuiv(m_shader->handle(), location, out.data());
        else
            static_assert(!sizeof(T), "Unsupported uniform element type for glGetUniform");
    }
};

/// \brief Test the \ref Shader RAII behavior.
///
/// The constructor should create a valid OpenGL shader program and the destructor should delete the program.
TEST_F(ShaderTest, ShaderRAII)
{
    GLuint originalID{};

    {
        const Shader original(VERTEX_SHADER_SRC, FRAGMENT_SHADER_SRC);
        originalID = original.handle();

        EXPECT_NE(originalID, 0);
        EXPECT_TRUE(static_cast<bool>(glIsProgram(originalID)));
    }

    EXPECT_FALSE(static_cast<bool>(glIsProgram(originalID)));
}

/// \brief Test the \ref Shader move constructor.
///
/// Move constructing a \ref Shader should preserve the underlying shader program ID.
TEST_F(ShaderTest, ShaderMoveConstructor)
{
    Shader original(VERTEX_SHADER_SRC, FRAGMENT_SHADER_SRC);
    const unsigned int originalID{ original.handle() };
    Shader moved{ std::move(original) };

    EXPECT_EQ(moved.handle(), originalID);
    EXPECT_TRUE(static_cast<bool>(glIsProgram(moved.handle())));
}

/// \brief Test the \ref Shader move assignment.
///
/// Move assignment should preserve the move source's shader program and clean up the resource held in the move target
/// before assigning the move source to it.
TEST_F(ShaderTest, ShaderMoveAssignment)
{
    Shader shader1(VERTEX_SHADER_SRC, FRAGMENT_SHADER_SRC);
    const unsigned int originalShader1ID{ shader1.handle() };
    Shader shader2(VERTEX_SHADER_SRC, FRAGMENT_SHADER_SRC);
    const unsigned int originalShader2ID{ shader2.handle() };

    shader2 = std::move(shader1);

    EXPECT_FALSE(static_cast<bool>(glIsProgram(originalShader2ID)));
    EXPECT_EQ(originalShader1ID, shader2.handle());
    EXPECT_TRUE(static_cast<bool>(glIsProgram(shader2.handle())));
}

/// \brief Test the \ref Shader move assignment when assigning to the same object.
///
/// When the move target is the same as the move source, nothing should change.
TEST_F(ShaderTest, ShaderMoveAssignmentOnSameShader)
{
    Shader shader(VERTEX_SHADER_SRC, FRAGMENT_SHADER_SRC);
    const unsigned int originalShaderID{ shader.handle() };

    shader = std::move(shader);

    EXPECT_TRUE(static_cast<bool>(glIsProgram(originalShaderID)));
    EXPECT_EQ(originalShaderID, shader.handle());
}

/// \brief Test the \ref Shader use method.
///
/// When the shader is being activated with the use method, OpenGL should report it back as the currently used program.
TEST_F(ShaderTest, ShaderUseActivatesProgram)
{
    m_shader->use();

    int activeProgram{ 0 };
    glGetIntegerv(GL_CURRENT_PROGRAM, &activeProgram);

    ASSERT_NE(0, activeProgram);
    EXPECT_EQ(activeProgram, m_shader->handle());
}

/// \brief Test uploading a 3 element float vector uniform value using a glm::vec3
///
/// When a value is uploaded to a uniform, the shader should receive the uploaded value
TEST_F(ShaderTest, SetFloatVector3ValueWithUse)
{
    const auto vec{
        glm::vec3(generateRandomValue<float>(), generateRandomValue<float>(), generateRandomValue<float>())
    };

    m_shader->setVector3f(FLOAT_3_UNIFORM, vec, true);

    std::array<GLfloat, glm::vec3::length()> uniformValues{};
    glGetUniformfv(m_shader->handle(), m_float3Location, uniformValues.data());

    EXPECT_FLOAT_EQ(vec.x, uniformValues.at(0));
    EXPECT_FLOAT_EQ(vec.y, uniformValues.at(1));
    EXPECT_FLOAT_EQ(vec.z, uniformValues.at(2));
}

/// \brief Test uploading a 4x4 float matrix uniform value
///
/// When a value is uploaded to a uniform, the shader should receive the uploaded value
TEST_F(ShaderTest, SetMatrix4ValueWithUse)
{
    const auto input{ glm::mat4(generateRandomValue<float>()) };

    m_shader->setMatrix4f(MATRIX_4_UNIFORM, input, true);

    std::array<GLfloat, static_cast<std::size_t>(glm::mat4::length() * glm::mat4::length())> uniformValues{};
    getUniformValue<GLfloat>(m_matrix4Location, uniformValues);

    const glm::mat4 output{ glm::make_mat4(uniformValues.data()) };
    EXPECT_EQ(input, output);
}

} // namespace pen::testing

