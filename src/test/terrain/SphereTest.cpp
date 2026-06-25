#include "terrain/Sphere.hpp"
#include "testUtility/OpenGLTestFixture.hpp"

#include <gtest/gtest.h>

#include <glm/glm.hpp>

#include <cstddef>
#include <memory>
#include <vector>

namespace
{

constexpr auto DELTA{ 1e-5f };

} // namespace

namespace pen::testing
{

/// \brief Test for the \ref Sphere class.
///
/// Private member access to \ref Sphere via friend class.
///
/// \author Felix Hommel
/// \date 6/25/2026
class SphereTest : public ::testing::Test
{
public:
    SphereTest() = default;
    ~SphereTest() override = default;

    SphereTest(const SphereTest&) = delete;
    SphereTest(SphereTest&&) = delete;
    SphereTest& operator=(const SphereTest&) = delete;
    SphereTest& operator=(SphereTest&&) = delete;

protected:
    [[nodiscard]] static const std::vector<Sphere::VertexPos>& vertices(const Sphere& s) { return s.m_vertices; }
    [[nodiscard]] static const std::vector<Sphere::VertexNorm>& normals(const Sphere& s) { return s.m_normals; }
    [[nodiscard]] static const std::vector<Sphere::VertexTexCoord>& texCoords(const Sphere& s) { return s.m_texCoords; }
    [[nodiscard]] static const std::vector<unsigned int>& indices(const Sphere& s) { return s.m_indices; }
    [[nodiscard]] static unsigned int vao(const Sphere& s) { return s.m_vao; }
    [[nodiscard]] static unsigned int vbo(const Sphere& s) { return s.m_vbo; }
    [[nodiscard]] static unsigned int ebo(const Sphere& s) { return s.m_ebo; }
};

/// \brief Constructing a \ref Sphere with zero sectors and stacks must clamp to the minimum (3 sectors, 2 stacks) and not crash.
TEST_F(SphereTest, MinimumSectorAndStackClamping)
{
    ASSERT_NO_FATAL_FAILURE({
        const Sphere s(1.f, 0u, 0u);

        EXPECT_FALSE(vertices(s).empty());
        EXPECT_FALSE(indices(s).empty());
    });
}

/// \brief \ref Sphere with a sector count below the minimum (3) should be clamped to 3 sectors.
TEST_F(SphereTest, BelowMinimumSectorsClampedToThree)
{
    const Sphere s(1.f, 2u, 4u);
    const auto expected{ static_cast<std::size_t>(4 + 1) * static_cast<std::size_t>(3 + 1) };

    EXPECT_EQ(vertices(s).size(), expected);
}

/// \brief \ref Sphere with a stack count below the minimum (2) should be clamped to 2 stacks.
TEST_F(SphereTest, BelowMinimumStacksClampedToThree)
{
    const Sphere s(1.f, 6u, 1u);
    const auto expected{ static_cast<std::size_t>(2 + 1) * static_cast<std::size_t>(6 + 1) };

    EXPECT_EQ(vertices(s).size(), expected);
}

/// \brief Test the properties and functions of a smooth \ref Sphere
///
/// \author Felix Hommel
/// \date 6/25/2026
class SmoothSphereTest : public SphereTest
{
public:
    SmoothSphereTest() = default;
    ~SmoothSphereTest() override = default;

    SmoothSphereTest(const SmoothSphereTest&) = delete;
    SmoothSphereTest(SmoothSphereTest&&) = delete;
    SmoothSphereTest& operator=(const SmoothSphereTest&) = delete;
    SmoothSphereTest& operator=(SmoothSphereTest&&) = delete;

protected:
    static constexpr auto RADIUS{ 2.f };
    static constexpr auto SECTORS{ 36u };
    static constexpr auto STACKS{ 18u };

    Sphere sphere{ RADIUS, SECTORS, STACKS, true };
};

/// \brief A smooth \ref Sphere must generate (stackCount + 1) * (sectorCount + 1) vertices.
TEST_F(SmoothSphereTest, VertexCount)
{
    const auto expected{ static_cast<std::size_t>(STACKS + 1) * static_cast<std::size_t>(SECTORS + 1) };

    EXPECT_EQ(vertices(sphere).size(), expected);
}

/// \brief The vertex, normal, and texCoord arrays must be the exact same length.
TEST_F(SmoothSphereTest, AttributeArraysSameLength)
{
    EXPECT_EQ(vertices(sphere).size(), normals(sphere).size());
    EXPECT_EQ(vertices(sphere).size(), texCoords(sphere).size());
}

/// \brief Every vertex must lie exactly on the sphere's surface (|v| == radius).
TEST_F(SmoothSphereTest, AllVerticesOnSurface)
{
    for(std::size_t i{ 0 }; i < vertices(sphere).size(); ++i)
    {
        const auto length{ glm::length(vertices(sphere)[i]) };

        EXPECT_NEAR(length, RADIUS, ::DELTA) << "Vertex " << i << " has |v|=" << length;
    }
}

/// \brief A smooth \ref Sphere must have normals that are equal to vertex / radius.
TEST_F(SmoothSphereTest, NormalsEqualVertexOverRadius)
{
    const auto& verts{ vertices(sphere) };
    const auto& norms{ normals(sphere) };

    for(std::size_t i{ 0 }; i < verts.size(); ++i)
    {
        const auto expected{ verts[i] / RADIUS };

        EXPECT_NEAR(norms[i].x, expected.x, ::DELTA) << "normal.x at vertex " << i;
        EXPECT_NEAR(norms[i].y, expected.y, ::DELTA) << "normal.y at vertex " << i;
        EXPECT_NEAR(norms[i].z, expected.z, ::DELTA) << "normal.z at vertex " << i;
    }
}

/// \brief Every stored normal must be a unit vector.
TEST_F(SmoothSphereTest, NormalsAreUnitLength)
{
    for(std::size_t i{ 0 }; i < normals(sphere).size(); ++i)
    {
        const auto length{ glm::length(normals(sphere)[i]) };

        EXPECT_NEAR(length, 1.f, ::DELTA) << "normal " << i << " has |n|=" << length;
    }
}

/// \brief All index values must be within the value vertex range.
TEST_F(SmoothSphereTest, IndicesWithinVertexRange)
{
    const auto vertCount{ static_cast<unsigned int>(vertices(sphere).size()) };

    for(std::size_t i{ 0 }; i < indices(sphere).size(); ++i)
        EXPECT_LT(indices(sphere)[i], vertCount) << "index " << i << " out of range";
}

/// \brief Index count must be a multiple of 3.
TEST_F(SmoothSphereTest, IndexCountDivisibleByThree)
{
    ASSERT_EQ(indices(sphere).size() % 3, 0u) << "indices are not a multiple of 3";
}

/// \brief A smooth \ref Sphere has all face normals pointing outward.
///     (dot(faceNormal, centroid) > 0 since the sphere is centered at origin).
TEST_F(SmoothSphereTest, FaceNormalsPointOutward)
{
    const auto& verts{ vertices(sphere) };
    const auto& idx{ indices(sphere) };

    for(std::size_t tri{ 0 }; tri < idx.size(); tri += 3)
    {
        const auto& p0{ verts[idx[tri]] };
        const auto& p1{ verts[idx[tri + 1]] };
        const auto& p2{ verts[idx[tri + 2]] };

        const auto faceNormal{ glm::cross(p1 - p0, p2 - p0) };
        const auto centroid{ (p0 + p1 + p2) / 3.f };

        if(glm::length(faceNormal) < ::DELTA)
            continue;

        EXPECT_GT(glm::dot(faceNormal, centroid), 0.f) << "inward-facing triangle at index " << tri / 3;
    }
}

/// \brief Texture coordinates must be in a [0, 1] x [0, 1] range.
TEST_F(SmoothSphereTest, TextureCoordsInUnitRange)
{
    for(std::size_t i{ 0 }; i < texCoords(sphere).size(); ++i)
    {
        EXPECT_GE(texCoords(sphere)[i].x, 0.f) << "u < 0 at " << i;
        EXPECT_LE(texCoords(sphere)[i].x, 1.f) << "u > 1 at " << i;
        EXPECT_GE(texCoords(sphere)[i].y, 0.f) << "v < 0 at " << i;
        EXPECT_LE(texCoords(sphere)[i].y, 1.f) << "v > 1 at " << i;
    }
}

/// \brief Test the properties and functions of a flat \ref Sphere
///
/// \author Felix Hommel
/// \date 6/25/2026
class FlatSphereTest : public SphereTest
{
public:
    FlatSphereTest() = default;
    ~FlatSphereTest() override = default;

    FlatSphereTest(const FlatSphereTest&) = delete;
    FlatSphereTest(FlatSphereTest&&) = delete;
    FlatSphereTest& operator=(const FlatSphereTest&) = delete;
    FlatSphereTest& operator=(FlatSphereTest&&) = delete;

protected:
    static constexpr auto RADIUS{ 1.f };
    static constexpr auto SECTORS{ 12u };
    static constexpr auto STACKS{ 6u };

    Sphere sphere{ RADIUS, SECTORS, STACKS, false };
};

/// \brief The vertex, normal, and texCoord arrays must be the exact same length.
TEST_F(FlatSphereTest, AttributeArraysSameLength)
{
    EXPECT_EQ(vertices(sphere).size(), normals(sphere).size());
    EXPECT_EQ(vertices(sphere).size(), texCoords(sphere).size());
}

/// \brief Every vertex must lie exactly on the sphere's surface (|v| == radius).
TEST_F(FlatSphereTest, AllVerticesOnSurface)
{
    for(std::size_t i{ 0 }; i < vertices(sphere).size(); ++i)
    {
        const auto length{ glm::length(vertices(sphere)[i]) };

        EXPECT_NEAR(length, RADIUS, ::DELTA);
    }
}

/// \brief Every stored normal must be a unit vector.
TEST_F(FlatSphereTest, NormalsAreUnitLength)
{
    for(std::size_t i{ 0 }; i < normals(sphere).size(); ++i)
    {
        const auto length{ glm::length(normals(sphere)[i]) };

        EXPECT_NEAR(length, 1.f, ::DELTA) << "normal " << i << " has |n|=" << length;
    }
}

/// \brief All index values must be within the value vertex range.
TEST_F(FlatSphereTest, IndicesWithinVertexRange)
{
    const auto vertCount{ static_cast<unsigned int>(vertices(sphere).size()) };

    for(std::size_t i{ 0 }; i < indices(sphere).size(); ++i)
        EXPECT_LT(indices(sphere)[i], vertCount) << "index " << i << " out of range";
}

/// \brief Index count must be a multiple of 3.
TEST_F(FlatSphereTest, IndexCountDivisibleByThree)
{
    ASSERT_EQ(indices(sphere).size() % 3, 0u) << "indices are not a multiple of 3";
}

/// \brief A flat shaded \ref Sphere needs to have three normals within a triangle that have the same value.
TEST_F(FlatSphereTest, PerTriangleNormalsAreConstant)
{
    const auto& norms{ normals(sphere) };
    const auto& idx{ indices(sphere) };

    for(std::size_t tri{ 0 }; tri < idx.size(); tri += 3)
    {
        const auto& n0{ norms[idx[tri]] };
        const auto& n1{ norms[idx[tri + 1]] };
        const auto& n2{ norms[idx[tri + 2]] };

        EXPECT_NEAR(n0.x, n1.x, ::DELTA) << "n0/n1 x missmatch at triangle " << tri / 3;
        EXPECT_NEAR(n0.y, n1.y, ::DELTA) << "n0/n1 y missmatch at triangle " << tri / 3;
        EXPECT_NEAR(n0.z, n1.z, ::DELTA) << "n0/n1 z missmatch at triangle " << tri / 3;
        EXPECT_NEAR(n0.x, n2.x, ::DELTA) << "n0/n1 x missmatch at triangle " << tri / 3;
        EXPECT_NEAR(n0.y, n2.y, ::DELTA) << "n0/n1 y missmatch at triangle " << tri / 3;
        EXPECT_NEAR(n0.z, n2.z, ::DELTA) << "n0/n1 z missmatch at triangle " << tri / 3;
    }
}

/// \brief Test the OpenGL functions of a \ref Sphere
///
/// \author Felix Hommel
/// \date 6/25/2026
class SphereGLTest : public SphereTest
{
public:
    SphereGLTest() = default;
    ~SphereGLTest() override = default;

    SphereGLTest(const SphereGLTest&) = delete;
    SphereGLTest(SphereGLTest&&) = delete;
    SphereGLTest& operator=(const SphereGLTest&) = delete;
    SphereGLTest& operator=(SphereGLTest&&) = delete;

    void SetUp() override
    {
        if(!m_glContext->setup())
            GTEST_SKIP() << m_glContext->getSkipReason();
    }

    void TearDown() override { m_glContext->teardown(); }

protected:
    static constexpr auto RADIUS{ 1.f };
    static constexpr auto SECTORS{ 12u };
    static constexpr auto STACKS{ 6u };

    std::unique_ptr<OpenGLTestFixture> m_glContext{ std::make_unique<OpenGLTestFixture>() };
};

/// \brief OpenGL must report the VAO, VBO, and EBO as valid objects after uploading the \ref Sphere data.
TEST_F(SphereGLTest, CopyToGPUCreatesNonZeroHandle)
{
    Sphere sphere(RADIUS, SECTORS, STACKS);

    sphere.copyToGPU();

    EXPECT_TRUE(static_cast<bool>(glIsVertexArray(vao(sphere))));
    EXPECT_TRUE(static_cast<bool>(glIsBuffer(vbo(sphere))));
    EXPECT_TRUE(static_cast<bool>(glIsBuffer(ebo(sphere))));
}

/// \brief The \ref draw() method may not trigger any high-severity OpenGL debug errors.
///
/// Because of the debug callback that is installed in \ref OpenGLTestFixture, tests would fail anyways for high-severity
/// messages but this additionally ensures that there are no other OpenGL errors pending.
TEST_F(SphereGLTest, DrawDoesNotProduceGLErrors)
{
    Sphere sphere(RADIUS, SECTORS, STACKS);

    sphere.copyToGPU();
    sphere.draw();

    EXPECT_EQ(glGetError(), static_cast<GLenum>(GL_NO_ERROR));
}

/// \brief After the \ref Sphere is destroyed, OpenGL must report it's buffer as gone.
TEST_F(SphereGLTest, DestructorDeletedGPUResources)
{
    GLuint savedVAO{ 0 };
    GLuint savedVBO{ 0 };
    GLuint savedEBO{ 0 };

    {
        const Sphere sphere(RADIUS, SECTORS, STACKS);

        savedVAO = vao(sphere);
        savedVBO = vbo(sphere);
        savedEBO = ebo(sphere);
    }

    EXPECT_FALSE(static_cast<bool>(glIsVertexArray(savedVAO)));
    EXPECT_FALSE(static_cast<bool>(glIsBuffer(savedVBO)));
    EXPECT_FALSE(static_cast<bool>(glIsBuffer(savedEBO)));
}

} // namespace pen::testing
