#ifndef PEN_SRC_ENGINE_TERRAIN_LATTICE_DATA_3D_HPP
#define PEN_SRC_ENGINE_TERRAIN_LATTICE_DATA_3D_HPP

#include <array>
#include <cstddef>

namespace pen
{

template<typename T, std::size_t X, std::size_t Y, std::size_t Z>
class LatticeData3D
{
public:
    LatticeData3D() : m_data{} {};
    LatticeData3D(const std::array<T, X * Y * Z>& data) : m_data{ data } {};
    ~LatticeData3D() = default;

    LatticeData3D(const LatticeData3D&) = default;
    LatticeData3D& operator=(const LatticeData3D&) = delete;
    LatticeData3D(LatticeData3D&&) = delete;
    LatticeData3D& operator=(LatticeData3D&&) = delete;

    T& operator[](std::size_t index) { return m_data[index]; }
    const T& operator[](std::size_t index) const { return m_data[index]; }

    T& operator[](const std::array<std::size_t, 3>& ijk) { return m_data[linearizeIndex(ijk[0], ijk[1], ijk[2])]; }
    const T& operator[](const std::array<std::size_t, 3>& ijk) const
    {
        return m_data[linearizeIndex(ijk[0], ijk[1], ijk[2])];
    }

    T& value(std::size_t i, std::size_t j, std::size_t k) { return m_data[linearizeIndex(i, j, k)]; }
    const T& value(std::size_t i, std::size_t j, std::size_t k) const { return m_data[linearizeIndex(i, j, k)]; }

protected:
    std::array<T, X * Y * Z> m_data;

    std::size_t linearizeIndex(std::size_t i, std::size_t j, std::size_t k) { return (i * Y * Z) + (j * Z) + k; }
};

} // namespace pen

#endif // !PEN_SRC_ENGINE_TERRAIN_LATTICE_DATA_3D_HPP
