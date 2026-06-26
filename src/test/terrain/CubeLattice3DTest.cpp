#include "terrain/CubeLattice3D.hpp"

#include <gtest/gtest.h>

#include <cstddef>

namespace
{

/// \brief Helper function to access the lattice at a certain spot.
///
/// \tparam X The size on the x-axis.
/// \tparam Y The size on the y-axis.
/// \tparam Z The size on the z-axis.
///
/// \param The lattice the lattice in which the position is looked up in.
/// \param The i-axis position.
/// \param The j-axis position.
/// \param The k-axis position.
///
/// \returns \ref glm::vec3 at (\p i, \p j, \p k).
template<std::size_t X, std::size_t Y, std::size_t Z>
glm::vec3 at(const pen::CubeLattice3D<X, Y, Z>& lattice, std::size_t i, std::size_t j, std::size_t k)
{
    return lattice[glm::ivec3(i, j, k)];
}

} // namespace

namespace pen::testing
{

/// \brief A 1x1x1 lattice has a single vertex that must sit exactly at the center.
TEST(CubeLattice3DConstruction, SinglePointLatticeAtCenter)
{
    constexpr auto spacing{ 1.f };
    constexpr auto center{ glm::vec3(3.f, -2.f, 5.f) };

    CubeLattice3D<1, 1, 1> lattice(spacing, center);

    const auto v{ at(lattice, 0, 0, 0) };

    EXPECT_FLOAT_EQ(v.x, center.x);
    EXPECT_FLOAT_EQ(v.y, center.y);
    EXPECT_FLOAT_EQ(v.z, center.z);
}

/// \brief Test the \ref CubeLattice3D origin properties.
///
/// \author Felix Hommel
/// \date 6/25/2026
class CubeLattice3DOriginTest : public ::testing::Test
{
public:
    CubeLattice3DOriginTest() = default;
    ~CubeLattice3DOriginTest() override = default;

    CubeLattice3DOriginTest(const CubeLattice3DOriginTest&) = delete;
    CubeLattice3DOriginTest(CubeLattice3DOriginTest&&) = delete;
    CubeLattice3DOriginTest& operator=(const CubeLattice3DOriginTest&) = delete;
    CubeLattice3DOriginTest& operator=(CubeLattice3DOriginTest&&) = delete;

protected:
    static constexpr std::size_t N{ 3 };
    static constexpr auto SPACING{ 1.f };

    CubeLattice3D<N, N, N> lattice{ SPACING, glm::vec3(0.f) };
};

/// \brief The corner vertex [0, 0, 0] must be at {-1, -1, -1} for a 3x3x3 lattice with spacing of 1 centered at the origin.
TEST_F(CubeLattice3DOriginTest, CornerOriginIsNegativeOne)
{
    const auto v{ at(lattice, 0, 0, 0) };

    EXPECT_FLOAT_EQ(v.x, -1.f);
    EXPECT_FLOAT_EQ(v.y, -1.f);
    EXPECT_FLOAT_EQ(v.z, -1.f);
}

/// \brief The corner vertex [N-1, N-1, N-1] must be at {1, 1, 1}.
TEST_F(CubeLattice3DOriginTest, CornerLastIsPositiveOne)
{
    const auto v{ at(lattice, N - 1, N - 1, N - 1) };

    EXPECT_FLOAT_EQ(v.x, 1.f);
    EXPECT_FLOAT_EQ(v.y, 1.f);
    EXPECT_FLOAT_EQ(v.z, 1.f);
}

/// \brief The central vertex [1, 1, 1] must lie exactly at the origin.
TEST_F(CubeLattice3DOriginTest, CenterVertexIsOrigin)
{
    const auto v{ at(lattice, 1, 1, 1) };

    EXPECT_FLOAT_EQ(v.x, 0.f);
    EXPECT_FLOAT_EQ(v.y, 0.f);
    EXPECT_FLOAT_EQ(v.z, 0.f);
}

/// \brief The lattice must be symmetric: vertex[i, j, k] == -vertex[N-1-i, N-1-j, N-1-k].
TEST_F(CubeLattice3DOriginTest, LatticeIsSymmetricAboutOrigin)
{
    for(std::size_t i{ 0 }; i < N; ++i)
    {
        for(std::size_t j{ 0 }; j < N; ++j)
        {
            for(std::size_t k{ 0 }; k < N; ++k)
            {
                const auto v{ at(lattice, i, j, k) };
                const auto mirror{ at(lattice, N - 1 - i, N - 1 - j, N - 1 - k) };

                EXPECT_FLOAT_EQ(v.x, -mirror.x) << "at (" << i << "," << j << "," << k << ")";
                EXPECT_FLOAT_EQ(v.y, -mirror.y) << "at (" << i << "," << j << "," << k << ")";
                EXPECT_FLOAT_EQ(v.z, -mirror.z) << "at (" << i << "," << j << "," << k << ")";
            }
        }
    }
}

/// \brief Consecutive vertices along the i-axis must differ by exactly gridSpace in x and be equal in y and z.
TEST(CubeLattice3DSpacing, UniformSpacingAlongIAxis)
{
    constexpr auto spacing{ 2.f };
    constexpr std::size_t X{ 4 };
    constexpr std::size_t Y{ 3 };
    constexpr std::size_t Z{ 2 };

    CubeLattice3D<X, Y, Z> lattice(spacing, glm::vec3(0.f));

    for(std::size_t i{ 0 }; i < X - 1; ++i)
    {
        const auto v0{ at(lattice, i, 0, 0) };
        const auto v1{ at(lattice, i + 1, 0, 0) };

        EXPECT_FLOAT_EQ(v1.x - v0.x, spacing) << "i=" << i;
        EXPECT_FLOAT_EQ(v1.y, v0.y) << "i=" << i;
        EXPECT_FLOAT_EQ(v1.z, v0.z) << "i=" << i;
    }
}

/// \brief Consecutive vertices along the j-axis must differ by exactly gridSpace in y and be equal in x and z.
TEST(CubeLattice3DSpacing, UniformSpacingAlongJAxis)
{
    constexpr auto spacing{ 0.5f };
    constexpr std::size_t X{ 2 };
    constexpr std::size_t Y{ 5 };
    constexpr std::size_t Z{ 2 };

    CubeLattice3D<X, Y, Z> lattice(spacing, glm::vec3(0.f));

    for(std::size_t j{ 0 }; j < Y - 1; ++j)
    {
        const auto v0{ at(lattice, 0, j, 0) };
        const auto v1{ at(lattice, 0, j + 1, 0) };

        EXPECT_FLOAT_EQ(v1.x, v0.x) << "j=" << j;
        EXPECT_FLOAT_EQ(v1.y - v0.y, spacing) << "j=" << j;
        EXPECT_FLOAT_EQ(v1.z, v0.z) << "j=" << j;
    }
}
/// \brief Consecutive vertices along the k-axis must differ by exactly gridSpace in z and be equal in x and y.
TEST(CubeLattice3DSpacing, UniformSpacingAlongKAxis)
{
    constexpr auto spacing{ 3.f };
    constexpr std::size_t X{ 2 };
    constexpr std::size_t Y{ 2 };
    constexpr std::size_t Z{ 6 };

    CubeLattice3D<X, Y, Z> lattice(spacing, glm::vec3(0.f));

    for(std::size_t j{ 0 }; j < Y - 1; ++j)
    {
        const auto v0{ at(lattice, 0, j, 0) };
        const auto v1{ at(lattice, 0, j + 1, 0) };

        EXPECT_FLOAT_EQ(v1.x, v0.x) << "j=" << j;
        EXPECT_FLOAT_EQ(v1.y - v0.y, spacing) << "j=" << j;
        EXPECT_FLOAT_EQ(v1.z, v0.z) << "j=" << j;
    }
}

/// \brief Shifting the center by a known vector must shift every vertex by the same amount.
TEST(CubeLattice3DCenter, OffsetShiftsAllVerticesUniformly)
{
    constexpr auto spacing{ 1.f };
    constexpr std::size_t N{ 3 };
    constexpr auto offset{ glm::vec3(7.f, -3.f, 2.f) };

    CubeLattice3D<N, N, N> latticeAt0(spacing, glm::vec3(0.f));
    CubeLattice3D<N, N, N> latticeAtOffset(spacing, offset);

    for(std::size_t i{ 0 }; i < N; ++i)
    {
        for(std::size_t j{ 0 }; j < N; ++j)
        {
            for(std::size_t k{ 0 }; k < N; ++k)
            {
                const auto v0{ at(latticeAt0, i, j, k) };
                const auto vOffset{ at(latticeAtOffset, i, j, k) };

                EXPECT_FLOAT_EQ(vOffset.x - v0.x, offset.x) << "at (" << i << "," << j << "," << k << ")";
                EXPECT_FLOAT_EQ(vOffset.y - v0.y, offset.y) << "at (" << i << "," << j << "," << k << ")";
                EXPECT_FLOAT_EQ(vOffset.z - v0.z, offset.z) << "at (" << i << "," << j << "," << k << ")";
            }
        }
    }
}

/// \brief Each axis component must use its own extent for the centering offset.
TEST(CubeLattice3DNonCubic, PerAxisCenteringOffset)
{
    constexpr auto spacing{ 1.f };
    constexpr std::size_t X{ 4 };
    constexpr std::size_t Y{ 2 };
    constexpr std::size_t Z{ 3 };
    constexpr auto adjust{ glm::vec3(spacing * (X - 1) / 2.f, spacing * (Y - 1) / 2.f, spacing * (Z - 1) / 2.f) };

    CubeLattice3D<X, Y, Z> lattice(spacing, glm::vec3(0.f));

    for(std::size_t i{ 0 }; i < X; ++i)
    {
        for(std::size_t j{ 0 }; j < Y; ++j)
        {
            for(std::size_t k{ 0 }; k < Z; ++k)
            {
                const auto v{ at(lattice, i, j, k) };
                const auto expectedX{ (static_cast<float>(i) * spacing) - adjust.x };
                const auto expectedY{ (static_cast<float>(j) * spacing) - adjust.y };
                const auto expectedZ{ (static_cast<float>(k) * spacing) - adjust.z };

                EXPECT_FLOAT_EQ(v.x, expectedX) << "x wrong at (" << i << "," << j << "," << k << ")";
                EXPECT_FLOAT_EQ(v.y, expectedY) << "x wrong at (" << i << "," << j << "," << k << ")";
                EXPECT_FLOAT_EQ(v.z, expectedZ) << "x wrong at (" << i << "," << j << "," << k << ")";
            }
        }
    }
}

} // namespace pen::testing
