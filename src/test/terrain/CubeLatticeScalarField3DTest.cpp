#include "terrain/CubeLatticeScalarField3D.hpp"

#include "terrain/LatticeData3D.hpp"

#include <gtest/gtest.h>

#include <glm/glm.hpp>

#include <algorithm>
#include <cstddef>

namespace
{

constexpr auto DELTA{ 1e-5f };

/// \brief Fill a \ref LatticeData3D object with a constant value.
///
/// \param field The \ref LatticeData3D object that is being filled
/// \param value The value that is assigned to \p field
template<std::size_t X, std::size_t Y, std::size_t Z>
void fillConstant(pen::LatticeData3D<float, X, Y, Z>& field, float value)
{
    for(std::size_t i{ 0 }; i < X * Y * Z; ++i)
        field[i] = value;
}

/// \brief Fill a \ref LatticeData3D object with sphere like data.
///
/// \param field The \ref LatticeData3D object that is being filled
/// \param radius The radius of the sphere
template<std::size_t X, std::size_t Y, std::size_t Z>
void fillSphereDensity(pen::LatticeData3D<float, X, Y, Z>& field, float radius)
{
    constexpr auto center{
        glm::vec3(static_cast<float>(X) / 2.f, static_cast<float>(Y) / 2.f, static_cast<float>(Z) / 2.f)
    };

    for(std::size_t i{ 0 }; i < X; ++i)
    {
        for(std::size_t j{ 0 }; j < Y; ++j)
        {
            for(std::size_t k{ 0 }; k < Z; ++k)
            {
                const auto pos{ glm::vec3(static_cast<float>(i), static_cast<float>(j), static_cast<float>(k)) };

                field[pos] = std::max(0.f, 1.f - (glm::distance(pos, center) / radius));
            }
        }
    }
}

} // namespace

namespace pen::testing
{

/// \brief When every scalar value is below the iso-level every cube index is 0, which maps to edge-mask 0 -> no triangles.
TEST(CubeLatticeScalarField3DEmptyOutput, AllBelowIsoLevelProducesEmptyOutput)
{
    constexpr std::size_t N{ 5 };

    LatticeData3D<float, N, N, N> field;
    fillConstant(field, 0.f);

    CubeLatticeScalarField3D<N, N, N> sdf(1.f, glm::vec3(0.f), field);

    EXPECT_TRUE(sdf.computeVertexDrawData(0.5).empty());
}

/// \brief When every scalar value is above the iso-level the cube index is 255, which also maps to edge-mask 0 -> no triangles.
TEST(CubeLatticeScalarField3DEmptyOutput, AllAboveIsoLevelProducesEmptyOutput)
{
    constexpr std::size_t N{ 20 };
    constexpr auto RADIUS{ 7.f };

    LatticeData3D<float, N, N, N> field;
    fillSphereDensity(field, RADIUS);

    CubeLatticeScalarField3D<N, N, N> sdf(1.f, glm::vec3(0.f), field);

    EXPECT_TRUE(sdf.computeVertexDrawData(0.5).empty());
}

/// \brief A sphere-like density field with iso-level 0.5 must produce some output.
TEST(CubeLatticeScalarField3DOutput, SphereDensityProducesNonEmptyOutput)
{
    constexpr std::size_t N{ 20 };
    constexpr auto RADIUS{ 7.f };

    LatticeData3D<float, N, N, N> field;
    fillSphereDensity(field, RADIUS);

    CubeLatticeScalarField3D<N, N, N> sdf(1.f, glm::vec3(0.f), field);

    EXPECT_FALSE(sdf.computeVertexDrawData(0.5).empty());
}

/// \brief The output size must be multiple of 18 (3 vertices * 6 floats each).
TEST(CubeLatticeScalarField3DOutput, OutputSizeIsMultipleOf18)
{
    constexpr std::size_t N{ 20 };
    constexpr auto RADIUS{ 7.f };

    LatticeData3D<float, N, N, N> field;
    fillSphereDensity(field, RADIUS);

    CubeLatticeScalarField3D<N, N, N> sdf(1.f, glm::vec3(0.f), field);
    const auto data{ sdf.computeVertexDrawData(0.5) };

    ASSERT_FALSE(data.empty());
    EXPECT_EQ(data.size() % 18, 0u) << "Output size " << data.size()
                                    << " is not a multiuple of 18 (3 vertices * 6 floats each)";
}

/// \brief Every stored normal (floats [3..5] of each 6-float vertex block) must be a unit vector within floating-point tolerance.
TEST(CubeLatticeScalarField3DNormals, StoredNormalsAreUnitLength)
{
    constexpr std::size_t N{ 20 };
    constexpr auto RADIUS{ 7.f };

    LatticeData3D<float, N, N, N> field;
    fillSphereDensity(field, RADIUS);

    CubeLatticeScalarField3D<N, N, N> sdf(1.f, glm::vec3(0.f), field);
    const auto data{ sdf.computeVertexDrawData(0.5) };

    ASSERT_FALSE(data.empty());

    constexpr std::size_t STRIDE{ 6 };
    constexpr std::size_t NORMAL_OFFSET{ 3 };

    for(std::size_t i{ 0 }; i < data.size(); i += STRIDE)
    {
        const auto normal{
            glm::vec3(data[i + NORMAL_OFFSET], data[i + NORMAL_OFFSET + 1], data[i + NORMAL_OFFSET + 2])
        };
        const float length{ glm::length(normal) };

        EXPECT_NEAR(length, 1.f, ::DELTA) << "Non-unit normal at vertex index " << (i / STRIDE);
    }
}

/// \brief For each triangle the stored normal must match the geometric normal computed from the three vertex positions (same winding).
TEST(CubeLatticeScalarField3DNormals, StoredNormalsMatchGeometricNormals)
{
    constexpr std::size_t N{ 20 };
    constexpr auto RADIUS{ 7.f };

    LatticeData3D<float, N, N, N> field;
    fillSphereDensity(field, RADIUS);

    CubeLatticeScalarField3D<N, N, N> sdf(1.f, glm::vec3(0.f), field);
    const auto data{ sdf.computeVertexDrawData(0.5) };

    ASSERT_FALSE(data.empty());
    ASSERT_EQ(data.size() % 18, 0u);

    constexpr std::size_t STRIDE{ 6 };
    constexpr std::size_t TRIANGLE_STRIDE{ STRIDE * 3 };

    for(std::size_t tri{ 0 }; tri < data.size(); tri += TRIANGLE_STRIDE)
    {
        auto readVertex = [&](std::size_t offset) -> glm::vec3 {
            return { data[tri + offset], data[tri + offset + 1], data[tri + offset + 2] };
        };

        const auto p0{ readVertex(0) };
        const auto p1{ readVertex(TRIANGLE_STRIDE) };
        const auto p2{ readVertex(TRIANGLE_STRIDE * 2) };

        const auto edge1{ p1 - p0 };
        const auto edge2{ p2 - p0 };
        const auto crossProd{ glm::cross(edge1, edge2) };
        const auto crossLength{ glm::length(crossProd) };

        if(crossLength < ::DELTA)
            continue;

        const auto geometricNormal{ crossProd / crossLength };

        for(std::size_t v{ 0 }; v < 3; ++v)
        {
            const auto storedNormal{ readVertex((TRIANGLE_STRIDE * v) + 3) };

            EXPECT_NEAR(storedNormal.x, geometricNormal.x, ::DELTA) << "triangle " << (tri / TRIANGLE_STRIDE);
            EXPECT_NEAR(storedNormal.y, geometricNormal.y, ::DELTA) << "triangle " << (tri / TRIANGLE_STRIDE);
            EXPECT_NEAR(storedNormal.z, geometricNormal.z, ::DELTA) << "triangle " << (tri / TRIANGLE_STRIDE);
        }
    }
}

/// \brief A higher iso-level on the same field should produce fewer triangles than a lower one (the iso-surface shrinks
///     as the threshold rises).
TEST(CubeLatticeScalarField3DIsoLevel, HigherIsoLvelProducesFewerTriangles)
{
    constexpr std::size_t N{ 20 };
    constexpr auto RADIUS{ 8.f };

    LatticeData3D<float, N, N, N> field;
    fillSphereDensity(field, RADIUS);

    CubeLatticeScalarField3D<N, N, N> sdf(1.f, glm::vec3(0.f), field);

    const auto dataLow{ sdf.computeVertexDrawData(0.2) };
    const auto dataHigh{ sdf.computeVertexDrawData(0.8) };

    ASSERT_FALSE(dataLow.empty()) << "Expected non-empty output at isoLevel 0.2";
    ASSERT_FALSE(dataHigh.empty()) << "Expected non-empty output at isoLevel 0.8";

    EXPECT_GT(dataLow.size(), dataHigh.size()) << "Expected more geometry at lower iso-level (larger surface)";
}

/// \brief \ref computeVertexDrawData() must be callable multiple times and return the same result.
TEST(CubeLatticeScalarField3DIsoLevel, RepeatedCallsReturnIdenticalData)
{
    constexpr std::size_t N{ 15 };
    constexpr auto RADIUS{ 5.f };
    constexpr auto ISO_LEVEL{ 0.5 };

    LatticeData3D<float, N, N, N> field;
    fillSphereDensity(field, RADIUS);

    CubeLatticeScalarField3D<N, N, N> sdf(1.f, glm::vec3(0.f), field);

    const auto first{ sdf.computeVertexDrawData(ISO_LEVEL) };
    const auto second{ sdf.computeVertexDrawData(ISO_LEVEL) };

    ASSERT_EQ(first.size(), second.size());

    for(std::size_t i{ 0 }; i < first.size(); ++i)
        EXPECT_FLOAT_EQ(first[i], second[i]) << "Mismatch at float index " << i;
}

} // namespace pen::testing
