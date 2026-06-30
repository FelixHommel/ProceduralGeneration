#ifndef PEN_SRC_ENGINE_TERRAIN_TERRAIN_SCALAR_FIELD_GENERATOR_HPP
#define PEN_SRC_ENGINE_TERRAIN_TERRAIN_SCALAR_FIELD_GENERATOR_HPP

#include "terrain/IScalarFieldGenerator.hpp"

#include <FastNoise/FastNoise.h>

#include <cstddef>
#include <memory>

namespace
{

namespace defaults
{

constexpr auto OCTAVES{ 5 };
constexpr auto GAIN{ 0.5f };
constexpr auto LACUNARITY{ 2.f };

} // namespace defaults

} // namespace

namespace pen
{

/// \brief Generate a scalar field that represents terrain generated from noise.
///
/// \tparam T Type of the data that is stored in the array.
/// \tparam X Size of the array on the x-axis.
/// \tparam Y Size of the array on the y-axis.
/// \tparam Z Size of the array on the z-axis.
///
/// \author Felix Hommel
/// \date 6/29/2026
template<std::size_t X, std::size_t Y, std::size_t Z>
class TerrainScalarFieldGenerator : public IScalarFieldGenerator<X, Y, Z>
{
public:
    /// \brief Create a new \ref TerrainScalarFieldGenerator with optional parameters.
    ///
    /// \param octaves (optional) Specify the amount of layers (octaves)
    /// \param gain (optional) Specify how much each octave contributes
    /// \param lacunarity (optional) Specify the frequency increase per octave
    explicit TerrainScalarFieldGenerator(
        int octaves = ::defaults::OCTAVES, float gain = ::defaults::GAIN, float lacunarity = ::defaults::LACUNARITY
    )
    {
        m_generator->SetSource(m_source);
        m_generator->SetOctaveCount(octaves);
        m_generator->SetGain(gain);
        m_generator->SetLacunarity(lacunarity);
    }
    ~TerrainScalarFieldGenerator() override = default;

    TerrainScalarFieldGenerator(const TerrainScalarFieldGenerator&) = delete;
    TerrainScalarFieldGenerator(TerrainScalarFieldGenerator&&) = delete;
    TerrainScalarFieldGenerator& operator=(const TerrainScalarFieldGenerator&) = delete;
    TerrainScalarFieldGenerator& operator=(TerrainScalarFieldGenerator&&) = delete;

    /// \brief Generate the scalar field.
    ///
    /// \param center The center of the scalar field
    ///
    /// \returns \ref LatticeData3D with the propagated scalar field
    [[nodiscard]] std::unique_ptr<LatticeData3D<float, X, Y, Z>> generate(
        [[maybe_unused]] const glm::vec3& center = ::defaults::CENTER
    ) const override
    {
        std::unique_ptr<LatticeData3D<float, X, Y, Z>> field{ std::make_unique<LatticeData3D<float, X, Y, Z>>() };

        m_generator->GenUniformGrid3D(field->data(), 0, 0, 0, X, Y, Z, 1.f, 1.f, 1.f, SEED);

        return field;
    };

private:
    static constexpr auto SEED{ 0xC0FFEE };

    FastNoise::SmartNode<FastNoise::Simplex> m_source{ FastNoise::New<FastNoise::Simplex>() };
    FastNoise::SmartNode<FastNoise::FractalFBm> m_generator{ FastNoise::New<FastNoise::FractalFBm>() };
};

} // namespace pen

#endif // !PEN_SRC_ENGINE_TERRAIN_TERRAIN_SCALAR_FIELD_GENERATOR_HPP
