#ifndef PEN_SRC_ENGINE_TERRAIN_LATTICE_DATA_3D_HPP
#define PEN_SRC_ENGINE_TERRAIN_LATTICE_DATA_3D_HPP

#include <glm/glm.hpp>

#include <array>
#include <cstddef>

namespace pen
{

/// \brief Class template wrapper for a 3D array of type \p T
///
/// Implements a 3D array in form of a flat 1D array with index shifting access methods.
///
/// \tparam T Type of the data that is stored in the array.
/// \tparam X Size of the array on the x-axis.
/// \tparam Y Size of the array on the y-axis.
/// \tparam Z Size of the array on the z-axis.
template<typename T, std::size_t X, std::size_t Y, std::size_t Z>
class LatticeData3D
{
public:
    LatticeData3D() : m_data{} {};
    ~LatticeData3D() = default;

    LatticeData3D(const LatticeData3D& other) = default;
    LatticeData3D& operator=(const LatticeData3D&) = delete;
    LatticeData3D(LatticeData3D&& other) noexcept = default;
    LatticeData3D& operator=(LatticeData3D&&) = delete;

    /// \brief Linear array access.
    T& operator[](std::size_t index) { return m_data[index]; }
    /// \brief Const linear array access.
    const T& operator[](std::size_t index) const { return m_data[index]; }

    /// \brief 3D array access.
    T& operator[](const glm::ivec3& ijk)
    {
        return m_data[linearizeIndex(
            static_cast<std::size_t>(ijk.x), static_cast<std::size_t>(ijk.y), static_cast<std::size_t>(ijk.z)
        )];
    }
    /// \brief Const 3D array access.
    const T& operator[](const glm::ivec3& ijk) const
    {
        return m_data[linearizeIndex(
            static_cast<std::size_t>(ijk.x), static_cast<std::size_t>(ijk.y), static_cast<std::size_t>(ijk.z)
        )];
    }

    /// \brief Value array access.
    T& value(std::size_t i, std::size_t j, std::size_t k) { return m_data[linearizeIndex(i, j, k)]; }
    /// \brief Const value array access.
    const T& value(std::size_t i, std::size_t j, std::size_t k) const { return m_data[linearizeIndex(i, j, k)]; }

    T* data() { return m_data.data(); }
    const T* data() const { return m_data.data(); }

protected:
    std::array<T, X * Y * Z> m_data;

    /// \brief Convert a 3D (i, j, k) index into an index into the linear array.
    ///
    /// \param i The position on the x-axis
    /// \param j The position on the y-axis
    /// \param k The position on the z-axis
    ///
    /// \returns linearized index based on the 3D index.
    [[nodiscard]] std::size_t linearizeIndex(std::size_t i, std::size_t j, std::size_t k) const noexcept
    {
        return (i * Y * Z) + (j * Z) + k;
    }
};

} // namespace pen

#endif // !PEN_SRC_ENGINE_TERRAIN_LATTICE_DATA_3D_HPP
