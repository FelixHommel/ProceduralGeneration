#include "Sphere.hpp"

#include "utility/Assert.hpp"

#include <glad/gl.h>
#include <glm/glm.hpp>

#include <cmath>
#include <cstddef>
#include <numbers>
#include <utility>
#include <vector>

namespace
{

constexpr auto MIN_SECTOR_COUNT{ 3 };
constexpr auto MIN_STACK_COUNT{ 2 };

constexpr auto VERTEX_VERTICES_COUNT{ pen::Sphere::VertexPos::length() };
constexpr auto VERTEX_NORMALS_COUNT{ pen::Sphere::VertexNorm::length() };
constexpr auto VERTEX_TEX_COORDS_COUNT{ pen::Sphere::VertexTexCoord::length() };
constexpr auto VERTEX_STRIDE{ sizeof(float)
                              * (::VERTEX_VERTICES_COUNT + ::VERTEX_NORMALS_COUNT + ::VERTEX_TEX_COORDS_COUNT) };

glm::vec3 computeFaceNormals(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3)
{
    constexpr auto EPSILON{ 0.000001f };

    const auto edge1{ v2 - v1 };
    const auto edge2{ v3 - v1 };

    const auto normal{ glm::cross(edge1, edge2) };
    const auto length{ glm::length(normal) };

    return (length > EPSILON) ? normal / length : glm::vec3{ 0.f };
}

} // namespace

namespace pen
{

Sphere::Sphere(float radius, unsigned int sectorCount, unsigned int stackCount, bool smooth)
    : m_radius{ radius }
    , m_sectorCount{ sectorCount > ::MIN_SECTOR_COUNT ? sectorCount : ::MIN_SECTOR_COUNT }
    , m_stackCount{ stackCount > ::MIN_STACK_COUNT ? stackCount : ::MIN_STACK_COUNT }
    , m_smooth{ smooth }
{
    if(m_smooth)
        buildVerticesSmooth();
    else
        buildVerticesFlat();
}

Sphere::~Sphere()
{
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vbo);
    glDeleteBuffers(1, &m_ebo);
}

void Sphere::draw() const
{
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices.size()), GL_UNSIGNED_INT, nullptr);
}

void Sphere::copyToGPU()
{
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(sizeof(Vertex) * m_interleavedVertices.size()),
        m_interleavedVertices.data(),
        GL_STATIC_DRAW
    );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(sizeof(unsigned int) * m_indices.size()),
        m_indices.data(),
        GL_STATIC_DRAW
    );

    glVertexAttribPointer(0, ::VERTEX_VERTICES_COUNT, GL_FLOAT, GL_FALSE, ::VERTEX_STRIDE, nullptr);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(
        1,
        ::VERTEX_NORMALS_COUNT,
        GL_FLOAT,
        GL_FALSE,
        ::VERTEX_STRIDE,
        reinterpret_cast<void*>(sizeof(float) * ::VERTEX_VERTICES_COUNT)
    );
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(
        2,
        ::VERTEX_TEX_COORDS_COUNT,
        GL_FLOAT,
        GL_FALSE,
        ::VERTEX_STRIDE,
        reinterpret_cast<void*>(sizeof(float) * (::VERTEX_VERTICES_COUNT + ::VERTEX_NORMALS_COUNT))
    );
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

/// \brief Construct a smooth \ref Sphere mesh
void Sphere::buildVerticesSmooth()
{
    const auto invLength{ 1.f / m_radius };
    const auto sectorStep{ (2.f * std::numbers::pi_v<float>) / static_cast<float>(m_sectorCount) };
    const auto stackStep{ std::numbers::pi_v<float> / static_cast<float>(m_stackCount) };

    for(unsigned int i{ 0 }; i <= m_stackCount; ++i)
    {
        const auto stackAngle{ (std::numbers::pi_v<float> / 2.f) - (static_cast<float>(i) * stackStep) };
        const auto stackSin{ std::sin(stackAngle) };
        const auto stackCos{ std::cos(stackAngle) };

        for(unsigned int j{ 0 }; j <= m_sectorCount; ++j)
        {
            const auto sectorAngle{ static_cast<float>(j) * sectorStep };

            const auto vertex{ glm::vec3(
                m_radius * stackCos * std::cos(sectorAngle),
                m_radius * stackCos * std::sin(sectorAngle),
                m_radius * stackSin
            ) };

            m_vertices.push_back(vertex);
            m_normals.push_back(vertex * invLength);
            m_texCoords.emplace_back(
                static_cast<float>(j) / static_cast<float>(m_sectorCount),
                static_cast<float>(i) / static_cast<float>(m_stackCount)
            );
        }
    }

    for(unsigned int i{ 0 }; i < m_stackCount; ++i)
    {
        for(unsigned int j{ 0 }; j < m_sectorCount; ++j)
        {
            const auto k1{ (i * (m_sectorCount + 1)) + j };
            const auto k2{ k1 + m_sectorCount + 1 };

            if(i != 0)
                addIndices({ k1, k2, k1 + 1 });

            if(i != (m_stackCount - 1))
                addIndices({ k1 + 1, k2, k2 + 1 });

            m_lineIndices.push_back(k1);
            m_lineIndices.push_back(k2);

            if(i != 0)
            {
                m_lineIndices.push_back(k1);
                m_lineIndices.push_back(k1 + 1);
            }
        }
    }

    buildInterleavedVertices();
}

/// \brief Construct a flat \ref Sphere mesh
void Sphere::buildVerticesFlat()
{
    struct TmpVertex
    {
        TmpVertex() = default;
        TmpVertex(glm::vec3 p, glm::vec2 t) : position{ std::move(p) }, texCoord{ std::move(t) } {}

        glm::vec3 position;
        glm::vec2 texCoord;
    };

    const float sectorStep{ (2.f * std::numbers::pi_v<float>) / static_cast<float>(m_sectorCount) };
    const float stackStep{ std::numbers::pi_v<float> / static_cast<float>(m_stackCount) };

    std::vector<TmpVertex> tmpVertices;
    for(unsigned int i{ 0 }; i <= m_stackCount; ++i)
    {
        const auto stackAngle{ (std::numbers::pi_v<float> / 2.f) - (static_cast<float>(i) * stackStep) };

        const float xy{ m_radius * std::cos(stackAngle) };
        const float z{ m_radius * std::sin(stackAngle) };

        for(unsigned int j{ 0 }; j <= m_sectorCount; ++j)
        {
            const auto sectorAngle{ static_cast<float>(j) * sectorStep };

            tmpVertices.emplace_back(
                glm::vec3(xy * std::cos(sectorAngle), xy * std::sin(sectorAngle), z),
                glm::vec2(
                    static_cast<float>(j) / static_cast<float>(m_sectorCount),
                    static_cast<float>(i) / static_cast<float>(m_stackCount)
                )
            );
        }
    }

    int index{ 0 };
    for(unsigned int i{ 0 }; i < m_stackCount; ++i)
    {
        auto vi1{ i * (static_cast<int>(m_sectorCount) + 1) };
        auto vi2{ (i + 1) * (static_cast<int>(m_sectorCount) + 1) };

        for(unsigned int j{ 0 }; j < m_sectorCount; ++j, ++vi1, ++vi2)
        {
            const auto& v1{ tmpVertices[vi1] };
            const auto& v2{ tmpVertices[vi2] };
            const auto& v3{ tmpVertices[static_cast<std::size_t>(vi1) + 1] };
            const auto& v4{ tmpVertices[static_cast<std::size_t>(vi2) + 1] };

            if(i == 0)
            {
                m_vertices.push_back(v1.position);
                m_vertices.push_back(v2.position);
                m_vertices.push_back(v4.position);

                m_texCoords.push_back(v1.texCoord);
                m_texCoords.push_back(v2.texCoord);
                m_texCoords.push_back(v4.texCoord);

                const auto normal{ ::computeFaceNormals(v1.position, v2.position, v4.position) };
                for(int k{ 0 }; k < 3; ++k) // NOTE: Add normal for each vertex
                    m_normals.push_back(normal);

                addIndices({ index, index + 1, index + 2 });

                m_lineIndices.push_back(index);
                m_lineIndices.push_back(index + 1);

                index += 3;
            }
            else if(i == static_cast<unsigned int>(m_stackCount - 1))
            {
                m_vertices.push_back(v1.position);
                m_vertices.push_back(v2.position);
                m_vertices.push_back(v3.position);

                m_texCoords.push_back(v1.texCoord);
                m_texCoords.push_back(v2.texCoord);
                m_texCoords.push_back(v3.texCoord);

                const auto normal{ ::computeFaceNormals(v1.position, v2.position, v3.position) };
                for(int k{ 0 }; k < 3; ++k) // NOTE: Add normal for each vertex
                    m_normals.push_back(normal);

                addIndices({ index, index + 1, index + 2 });

                m_lineIndices.push_back(index);
                m_lineIndices.push_back(index + 1);
                m_lineIndices.push_back(index);
                m_lineIndices.push_back(index + 2);

                index += 3;
            }
            else
            {
                m_vertices.push_back(v1.position);
                m_vertices.push_back(v2.position);
                m_vertices.push_back(v3.position);
                m_vertices.push_back(v4.position);

                m_texCoords.push_back(v1.texCoord);
                m_texCoords.push_back(v2.texCoord);
                m_texCoords.push_back(v3.texCoord);
                m_texCoords.push_back(v4.texCoord);

                const auto normal{ ::computeFaceNormals(v1.position, v2.position, v3.position) };
                for(int k{ 0 }; k < 4; ++k) // NOLINT(readability-magic-numbers) // NOTE: Add normal for each vertex
                    m_normals.push_back(normal);

                addIndices({ index, index + 1, index + 2 });
                addIndices({ index + 2, index + 1, index + 3 });

                m_lineIndices.push_back(index);
                m_lineIndices.push_back(index + 1);
                m_lineIndices.push_back(index);
                m_lineIndices.push_back(index + 2);

                index += 4; // NOLINT(readability-magic-numbers)
            }
        }
    }

    buildInterleavedVertices();
}

/// \brief Combine the individual vertex attributes into a single array of vertex data.
void Sphere::buildInterleavedVertices()
{
    PEN_ASSERT(m_vertices.size() == m_normals.size(), "Vertex attribute size mismatch");
    PEN_ASSERT(m_vertices.size() == m_texCoords.size(), "Vertex attribute size mismatch");

    for(std::size_t i{ 0 }; i < m_vertices.size(); ++i)
        m_interleavedVertices.emplace_back(m_vertices[i], m_normals[i], m_texCoords[i]);
}

void Sphere::addIndices(const Index& index)
{
    m_indices.push_back(index.x);
    m_indices.push_back(index.y);
    m_indices.push_back(index.z);
}

} // namespace pen
