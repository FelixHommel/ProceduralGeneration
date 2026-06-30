#ifndef PEN_SRC_ENGINE_TERRAIN_I_SCALAR_FIELD_GENERATOR_HPP
#define PEN_SRC_ENGINE_TERRAIN_I_SCALAR_FIELD_GENERATOR_HPP

#include "terrain/LatticeData3D.hpp"

#include <glm/glm.hpp>

#include <cstddef>
#include <memory>

namespace
{

namespace defaults
{

constexpr auto CENTER{ glm::vec3(0.f) };

} // namespace defaults

} // namespace

namespace pen
{

/// \brief Interface specification for scalar field generators.
///
/// \tparam T Type of the data that is stored in the array.
/// \tparam X Size of the array on the x-axis.
/// \tparam Y Size of the array on the y-axis.
/// \tparam Z Size of the array on the z-axis.
///
/// \author Felix Hommel
/// \date 6/29/2026
template<std::size_t X, std::size_t Y, std::size_t Z>
class IScalarFieldGenerator
{
public:
    IScalarFieldGenerator() = default;
    virtual ~IScalarFieldGenerator() = default;

    IScalarFieldGenerator(const IScalarFieldGenerator&) = delete;
    IScalarFieldGenerator& operator=(const IScalarFieldGenerator&) = delete;
    IScalarFieldGenerator(IScalarFieldGenerator&&) = delete;
    IScalarFieldGenerator& operator=(IScalarFieldGenerator&&) = delete;

    /// \brief Generate the scalar field.
    ///
    /// \param center The center of the scalar field
    ///
    /// \returns \ref LatticeData3D with the propagated scalar field
    [[nodiscard]] virtual std::unique_ptr<LatticeData3D<float, X, Y, Z>> generate(const glm::vec3& center) const = 0;
};

} // namespace pen

#endif // !PEN_SRC_ENGINE_TERRAIN_I_SCALAR_FIELD_GENERATOR_HPP
