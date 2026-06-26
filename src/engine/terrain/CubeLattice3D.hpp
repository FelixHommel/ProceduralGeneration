#ifndef PEN_SRC_ENGINE_TERAIN_CUBE_LATTICE_3D_HPP
#define PEN_SRC_ENGINE_TERAIN_CUBE_LATTICE_3D_HPP

#include "terrain/LatticeData3D.hpp"

#include <glm/glm.hpp>

#include <cstddef>

namespace pen
{

template<std::size_t X, std::size_t Y, std::size_t Z>
class CubeLattice3D : public LatticeData3D<glm::vec3, X, Y, Z>
{
public:
    CubeLattice3D(float gridSpace, const glm::vec3& center)
        : m_gridSpace{ gridSpace }, m_center{ center }, LatticeData3D<glm::vec3, X, Y, Z>{}
    {
        computeLatticeVertices();
    }
    ~CubeLattice3D() = default;

    CubeLattice3D(const CubeLattice3D&) = delete;
    CubeLattice3D& operator=(const CubeLattice3D&) = delete;
    CubeLattice3D(CubeLattice3D&&) = delete;
    CubeLattice3D& operator=(CubeLattice3D&&) = delete;

private:
    float m_gridSpace;
    const glm::vec3 m_center;

    void computeLatticeVertices()
    {
        const auto adjust{ glm::vec3(
            (-m_gridSpace * static_cast<float>(X - 1) / 2.f) + m_center.x,
            (-m_gridSpace * static_cast<float>(Y - 1) / 2.f) + m_center.y,
            (-m_gridSpace * static_cast<float>(Z - 1) / 2.f) + m_center.z
        ) };

        for(std::size_t i{ 0 }; i < X; ++i)
        {
            for(std::size_t j{ 0 }; j < Y; ++j)
            {
                for(std::size_t k{ 0 }; k < Z; ++k)
                {
                    this->value(i, j, k) = {
                        (static_cast<float>(i) * m_gridSpace) + adjust.x,
                        (static_cast<float>(j) * m_gridSpace) + adjust.y,
                        (static_cast<float>(k) * m_gridSpace) + adjust.z,
                    };
                }
            }
        }
    }
};

} // namespace pen

#endif // !PEN_SRC_ENGINE_TERAIN_CUBE_LATTICE_3D_HPP
