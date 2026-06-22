#ifndef PEN_SRC_ENGINE_TERRAIN_SPHERE_HPP
#define PEN_SRC_ENGINE_TERRAIN_SPHERE_HPP

#include <glm/glm.hpp>

#include <vector>

namespace
{

namespace defaults
{

constexpr auto RADIUS{ 1.f };
constexpr auto SECTOR_COUNT{ 36u };
constexpr auto STACK_COUNT{ 18u };
constexpr auto SMOOTHNESS{ true };
constexpr auto INTERLEAVED_STRIDE{ 32 };

} // namespace defaults

} // namespace

namespace pen
{

/// \brief
///
/// \author Felix Hommel
/// \date 6/22/2026
///
/// \note Adaption of https://www.songho.ca/opengl/gl_sphere.html
class Sphere
{
public:
    explicit Sphere(
        float radius = ::defaults::RADIUS,
        unsigned int sectorCount = ::defaults::SECTOR_COUNT,
        unsigned int stackCount = ::defaults::STACK_COUNT,
        bool smooth = ::defaults::SMOOTHNESS
    );
    ~Sphere();

    Sphere(const Sphere&) = delete;
    Sphere& operator=(const Sphere&) = delete;
    Sphere(Sphere&&) = delete;
    Sphere& operator=(Sphere&&) = delete;

    void draw() const;
    void copyToGPU();

    void clearSphereData();

private:
    float m_radius;
    unsigned int m_sectorCount;
    unsigned int m_stackCount;
    bool m_smooth;

    std::vector<float> m_vertices;
    std::vector<float> m_normals;
    std::vector<float> m_texCoords;
    std::vector<unsigned int> m_indices;
    std::vector<unsigned int> m_lineIndices;

    std::vector<float> m_interleavedVertices;
    int m_interleavedStride{ ::defaults::INTERLEAVED_STRIDE };

    unsigned int m_vao{ 0u };

    void buildVerticesSmooth();
    void buildVerticesFlat();
    void buildInterleavedVertices();
    void addVertex(const glm::vec3& vertex);
    void addNormal(const glm::vec3& normal);
    void addTexCoord(const glm::vec2& texCoord);
    void addIndices(const glm::vec<3, unsigned int>& index); // NOLINT(readability-magic-numbers)
};

} // namespace pen

#endif // !PEN_SRC_ENGINE_TERRAIN_SPHERE_HPP
