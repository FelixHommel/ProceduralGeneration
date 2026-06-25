#include <gtest/gtest.h>

#include "terrain/LatticeData3D.hpp"
#include "testUtility/RandomNumberGenerator.hpp"

#include <cstddef>

namespace
{

namespace defaults
{

constexpr std::size_t DIM_X{ 4 };
constexpr std::size_t DIM_Y{ 5 };
constexpr std::size_t DIM_Z{ 6 };

} // namespace defaults

/// \brief Generate a random index into the \ref LatticeData3D.
///
/// \param x The size on the x-axis
/// \param y The size on the y-axis
/// \param z The size on the z-axis
///
/// \returns \ref glm::ivec3 that is within the bounds of the \ref LatticeData3D
glm::ivec3 generateRandomIndex(std::size_t x, std::size_t y, std::size_t z)
{
    return {
        pen::testing::generateRandomValue<std::size_t>(0, x - 1),
        pen::testing::generateRandomValue<std::size_t>(0, y - 1),
        pen::testing::generateRandomValue<std::size_t>(0, z - 1),
    };
}

} // namespace

namespace pen::testing
{

/// \brief Thin Subclass for a \ref LatticeData3D that is used to expose protected members and functions for the sake of
///     testing.
///
/// \author Felix Hommel
/// \date 6/25/2026
template<typename T, std::size_t X, std::size_t Y, std::size_t Z>
class LatticeData3DExposed : public LatticeData3D<T, X, Y, Z>
{
public:
    using LatticeData3D<T, X, Y, Z>::linearizeIndex;
    using LatticeData3D<T, X, Y, Z>::m_data;
};

using DefaultSizeLattice = LatticeData3DExposed<int, defaults::DIM_X, defaults::DIM_Y, defaults::DIM_Z>;

/// \brief The origin index [0, 0, 0] must linearize to 0.
TEST(LatticeData3DLinearizeIndex, OriginIsZero)
{
    DefaultSizeLattice latice;

    EXPECT_EQ(latice.linearizeIndex(0, 0, 0), 0u);
}

/// \brief The last index [X-1, Y-1, Z-1] must linearize to X*Y*Z - 1.
TEST(LatticeData3DLinearizeIndex, LastIndexIsXYZMinusOne)
{
    DefaultSizeLattice lattice;

    EXPECT_EQ(
        lattice.linearizeIndex(::defaults::DIM_X - 1, ::defaults::DIM_Y - 1, ::defaults::DIM_Z - 1),
        (::defaults::DIM_X * ::defaults::DIM_Y * ::defaults::DIM_Z) - 1
    );
}

/// \brief Stepping i by 1 must advance the linear index by Y * Z.
TEST(LatticeData3DLinearizeIndex, StepInIAdvancesByYTimesZ)
{
    DefaultSizeLattice lattice;

    for(std::size_t i{ 0 }; i < ::defaults::DIM_X - 1; ++i)
    {
        EXPECT_EQ(
            lattice.linearizeIndex(i + 1, 0, 0) - lattice.linearizeIndex(i, 0, 0), ::defaults::DIM_Y * ::defaults::DIM_Z
        );
    }
}

/// \brief Stepping j by 1 must advance the linear index by Z.
TEST(LatticeData3DLinearizeIndex, StepInJAdvancesByZ)
{
    DefaultSizeLattice lattice;

    for(std::size_t j{ 0 }; j < ::defaults::DIM_Y - 1; ++j)
        EXPECT_EQ(lattice.linearizeIndex(0, j + 1, 0) - lattice.linearizeIndex(0, j, 0), ::defaults::DIM_Z);
}

/// \brief Stepping k by 1 must advance the linear index by exactly 1.
TEST(LatticeData3DLinearizeIndex, StepInKAdvancesByOne)
{
    DefaultSizeLattice lattice;

    for(std::size_t k{ 0 }; k < ::defaults::DIM_Z - 1; ++k)
        EXPECT_EQ(lattice.linearizeIndex(0, 0, k + 1) - lattice.linearizeIndex(0, 0, k), 1u);
}

/// \brief A known mid-point index must equal (i * Y * Z) + (j * Z) + k.
TEST(LatticeData3DLinearizeIndex, KnownMidpoint)
{
    DefaultSizeLattice lattice;

    const auto idx{ ::generateRandomIndex(::defaults::DIM_X, ::defaults::DIM_Y, ::defaults::DIM_Z) };

    EXPECT_EQ(
        lattice.linearizeIndex(idx.x, idx.y, idx.z),
        (idx.x * ::defaults::DIM_Y * ::defaults::DIM_Z) + (idx.y * ::defaults::DIM_Z) + idx.z
    );
}

/// \brief Writing through the linear operator and reading back must round-trip.
TEST(LatticeData3DLinearAccess, WriteReadRoundtrip)
{
    constexpr auto factor{ 10 };

    LatticeData3D<int, 3, 3, 3> lattice;

    for(std::size_t idx{ 0 }; idx < static_cast<std::size_t>(3 * 3 * 3); ++idx)
        lattice[idx] = static_cast<int>(idx * factor);

    for(std::size_t idx{ 0 }; idx < static_cast<std::size_t>(3 * 3 * 3); ++idx)
        EXPECT_EQ(lattice[idx], static_cast<int>(idx * factor));
}

/// \brief The const version of operator[] must return the same value that was written.
TEST(LatticeData3DLinearAccess, ConstAccessReturnsCorrectValue)
{
    LatticeData3D<int, 2, 2, 2> lattice;

    const auto latticePos{ generateRandomValue<std::size_t>(0, (2 * 2 * 2) - 1) };
    const auto latticePosValue{ generateRandomValue<int>() };
    lattice[latticePos] = latticePosValue;

    const auto& cLattice{ lattice };

    EXPECT_EQ(cLattice[latticePos], latticePosValue);
}

/// \brief Writing through the 3D operator and reading back must round-trip.
TEST(LatticeData3D3DAccess, WriteReadRoundtrip)
{
    LatticeData3D<float, 4, 3, 2> lattice; // NOLINT(readability-magic-numbers)
    const auto idx{ ::generateRandomIndex(4, 3, 2) };
    const auto value{ generateRandomValue<float>() };

    lattice[idx] = value;

    EXPECT_EQ(lattice[idx], value);
}

/// \brief The 3D operator must address the same underlying elements as the linear operator with the equivalent flat index.
TEST(LatticeData3D3DAccess, AliasesLinearOperator)
{
    constexpr std::size_t X{ 4 };
    constexpr std::size_t Y{ 3 };
    constexpr std::size_t Z{ 2 };

    LatticeData3D<int, X, Y, Z> lattice;

    for(std::size_t n{ 0 }; n < X * Y * Z; ++n)
        lattice[n] = static_cast<int>(n);

    for(std::size_t i{ 0 }; i < X; ++i)
    {
        for(std::size_t j{ 0 }; j < Y; ++j)
        {
            for(std::size_t k{ 0 }; k < Z; ++k)
            {
                const auto expected{ (i * Y * Z) + (j * Z) + k };

                EXPECT_EQ(lattice[glm::ivec3(i, j, k)], static_cast<int>(expected))
                    << "at (" << i << "," << j << "," << k << ")";
            }
        }
    }
}

/// \brief The const 3D operator must return the same value that was written.
TEST(LatticeData3D3DAccess, ConstAccessReturnsCorrectValue)
{
    LatticeData3D<double, 2, 2, 2> lattice;
    const auto idx{ ::generateRandomIndex(2, 2, 2) };
    const auto value{ generateRandomValue<double>() };

    lattice[idx] = value;

    const auto& cLattice{ lattice };

    EXPECT_DOUBLE_EQ(cLattice[idx], value);
}

/// \brief \ref LatticeData3D::value() must alias the same storage as \ref LatticeData3D::operator[](glm::ivec3).
TEST(LatticeData3DValueAccess, AliasesArrayOperator)
{
    LatticeData3D<int, 3, 3, 3> lattice;

    for(std::size_t i{ 0 }; i < 3; ++i)
        for(std::size_t j{ 0 }; j < 3; ++j)
            for(std::size_t k{ 0 }; k < 3; ++k)
                lattice.value(i, j, k) = static_cast<int>((i * 3 * 3) + (j * 3) + k);


    for(std::size_t i{ 0 }; i < 3; ++i)
    {
        for(std::size_t j{ 0 }; j < 3; ++j)
        {
            for(std::size_t k{ 0 }; k < 3; ++k)
            {
                EXPECT_EQ(lattice[glm::ivec3(i, j, k)], lattice.value(i, j, k))
                    << "at (" << i << "," << j << "," << k << ")";
            }
        }
    }
}

/// \brief Writing through \ref LatticeData3D::value() must be visible through \ref LatticeData3D::operator[](glm::ivec3).
TEST(LatticeData3DValueAccess, WriteThroughValueVisibleViaLinearOperator)
{
    constexpr std::size_t X{ 2 };
    constexpr std::size_t Y{ 3 };
    constexpr std::size_t Z{ 4 };

    const auto idx{ ::generateRandomIndex(X, Y, Z) };
    const auto value{ generateRandomValue<int>() };
    const auto linearIndex{ (idx.x * Y * Z) + (idx.y * Z) + idx.z };

    LatticeData3D<int, X, Y, Z> lattice;

    lattice.value(idx.x, idx.y, idx.z) = value;

    EXPECT_EQ(lattice[linearIndex], value);
}

/// \brief Writing through \ref LatticeData3D::operator[](std::size_t).
TEST(LatticeData3DValueAccess, WriteThroughValueVisibleViaValue)
{
    constexpr std::size_t X{ 2 };
    constexpr std::size_t Y{ 3 };
    constexpr std::size_t Z{ 4 };

    const auto idx{ ::generateRandomIndex(X, Y, Z) };
    const auto value{ generateRandomValue<int>() };
    const auto linearIndex{ (idx.x * Y * Z) + (idx.y * Z) + idx.z };

    LatticeData3D<int, X, Y, Z> lattice;

    lattice[linearIndex] = value;
    EXPECT_EQ(lattice.value(idx.x, idx.y, idx.z), value);
}

/// \brief The const \ref LatticeData3D::value() must return the stored value.
TEST(LatticeData3DValueAccess, ConstValueReturnsCorrectValue)
{
    const auto idx{ ::generateRandomIndex(2, 2, 2) };
    const auto value{ generateRandomValue<int>() };

    LatticeData3D<int, 2, 2, 2> lattice;

    lattice.value(idx.x, idx.y, idx.z) = value;

    const auto& cLattice{ lattice };

    EXPECT_EQ(cLattice.value(idx.x, idx.y, idx.z), value);
}

/// \brief Default-constructed lattice must hold the zero-value for its type.
TEST(LatticeData3DConstruction, DefaultConstructedIsZero)
{
    constexpr std::size_t X{ 3 };
    constexpr std::size_t Y{ 4 };
    constexpr std::size_t Z{ 5 };

    LatticeData3D<int, X, Y, Z> lattice;

    for(std::size_t i{ 0 }; i < X * Y * Z; ++i)
        EXPECT_EQ(lattice[i], 0);
}

} // namespace pen::testing
