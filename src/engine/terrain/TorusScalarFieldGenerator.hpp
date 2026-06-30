#ifndef PEN_SRC_ENGINE_TERRAIN_TORUS_SCALAR_FIELD_GENERATOR_HPP
#define PEN_SRC_ENGINE_TERRAIN_TORUS_SCALAR_FIELD_GENERATOR_HPP

#include "terrain/IScalarFieldGenerator.hpp"
#include "terrain/LatticeData3D.hpp"

#include <glm/glm.hpp>

#include <cmath>
#include <cstddef>
#include <memory>

namespace
{

namespace defaults
{

constexpr auto OUTER_RADIUS{ 8.f };
constexpr auto INNER_RADIUS{ 3.f };

} // namespace defaults

} // namespace

namespace pen
{

/// \brief Generate a scalar field that represents a torus shape.
///
/// \tparam T Type of the data that is stored in the array.
/// \tparam X Size of the array on the x-axis.
/// \tparam Y Size of the array on the y-axis.
/// \tparam Z Size of the array on the z-axis.
///
/// \author Felix Hommel
/// \date 6/29/2026
template<std::size_t X, std::size_t Y, std::size_t Z>
class TorusScalarFieldGenerator : public IScalarFieldGenerator<X, Y, Z>
{
public:
    /// \brief Create a new generator.
    ///
    /// \param outer The outer radius of the Torus
    /// \param inner The inner radius of the Torus
    explicit TorusScalarFieldGenerator(float outer = ::defaults::OUTER_RADIUS, float inner = ::defaults::INNER_RADIUS)
        : m_outer{ outer }, m_inner{ inner }
    {}
    ~TorusScalarFieldGenerator() override = default;

    TorusScalarFieldGenerator(const TorusScalarFieldGenerator&) = delete;
    TorusScalarFieldGenerator(TorusScalarFieldGenerator&&) = delete;
    TorusScalarFieldGenerator& operator=(const TorusScalarFieldGenerator&) = delete;
    TorusScalarFieldGenerator& operator=(TorusScalarFieldGenerator&&) = delete;

    /// \brief Generate the scalar field.
    ///
    /// \param center The center of the scalar field
    ///
    /// \returns \ref LatticeData3D with the propagated scalar field
    [[nodiscard]] std::unique_ptr<LatticeData3D<float, X, Y, Z>> generate(
        const glm::vec3& center = ::defaults::CENTER
    ) const override
    {
        std::unique_ptr<LatticeData3D<float, X, Y, Z>> field{ std::make_unique<LatticeData3D<float, X, Y, Z>>() };

        for(std::size_t i = 0; i < X; ++i)
        {
            for(std::size_t j = 0; j < Y; ++j)
            {
                for(std::size_t k = 0; k < Z; ++k)
                {
                    const auto x{ static_cast<float>(i) - (static_cast<float>(X) / 2.f) + center.x };
                    const auto y{ static_cast<float>(j) - (static_cast<float>(Y) / 2.f) + center.y };
                    const auto z{ static_cast<float>(k) - (static_cast<float>(Z) / 2.f) + center.z };

                    const auto distToRing{ std::sqrt(
                        ((std::sqrt((x * x) + (z * z)) - m_outer) * (std::sqrt((x * x) + (z * z)) - m_outer)) + (y * y)
                    ) };

                    (*field)[{ i, j, k }] = std::max(0.f, 1.f - (distToRing / m_inner));
                }
            }
        }

        return field;
    }

private:
    const float m_outer;
    const float m_inner;
};

} // namespace pen

#endif // !PEN_SRC_ENGINE_TERRAIN_TORUS_SCALAR_FIELD_GENERATOR_HPP
