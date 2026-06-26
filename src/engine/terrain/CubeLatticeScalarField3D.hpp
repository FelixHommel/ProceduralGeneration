#ifndef PEN_SRC_ENGINE_TERRAIN_CUBE_LATTICE_SCALAR_FIELD_3D_HPP
#define PEN_SRC_ENGINE_TERRAIN_CUBE_LATTICE_SCALAR_FIELD_3D_HPP

#include "terrain/CubeLattice3D.hpp"
#include "terrain/LatticeData3D.hpp"
#include "terrain/MarchingCubesData.hpp"

#include <glm/glm.hpp>

#include <array>
#include <cstddef>
#include <utility>
#include <vector>

namespace pen
{

struct Triangle
{
public:
    Triangle() : m_vertices{} {}
    Triangle(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3) : m_vertices{ p1, p2, p3 } {}
    explicit Triangle(const std::array<glm::vec3, 3>& points) : m_vertices{ points } {}
    ~Triangle() = default;

    Triangle(const Triangle&) = default;
    Triangle& operator=(const Triangle&) = default;
    Triangle(Triangle&&) = delete;
    Triangle& operator=(Triangle&&) = delete;

    glm::vec3& operator[](std::size_t index) { return m_vertices.at(index); }
    const glm::vec3& operator[](std::size_t index) const { return m_vertices.at(index); }

    glm::vec3 computeNormal()
    {
        const auto dir{ glm::cross(m_vertices[1] - m_vertices[0], m_vertices[2] - m_vertices[0]) };

        return dir / glm::length(dir);
    }

private:
    std::array<glm::vec3, 3> m_vertices;
};

template<std::size_t X, std::size_t Y, std::size_t Z>
class CubeLatticeScalarField3D
{
public:
    CubeLatticeScalarField3D(float spacing, const glm::vec3& center) : m_cubeLattice{ spacing, center }, m_scalarField{}
    {}
    CubeLatticeScalarField3D(float spacing, const glm::vec3& center, LatticeData3D<float, X, Y, Z>& scalarField)
        : m_cubeLattice{ spacing, center }, m_scalarField{ scalarField }
    {}
    ~CubeLatticeScalarField3D() = default;

    CubeLatticeScalarField3D(const CubeLatticeScalarField3D&) = delete;
    CubeLatticeScalarField3D& operator=(const CubeLatticeScalarField3D&) = delete;
    CubeLatticeScalarField3D(CubeLatticeScalarField3D&&) = delete;
    CubeLatticeScalarField3D& operator=(CubeLatticeScalarField3D&&) = delete;

    std::vector<float> computeVertexDrawData(double isoLevel)
    {
        std::vector<float> result;
        std::array<Triangle, 5> triangleAfterPolygonize; // NOLINT(readability-magic-numbers)

        for(std::size_t i{ 0 }; i < X - 1; ++i)
        {
            for(std::size_t j{ 0 }; j < Y - 1; ++j)
            {
                for(std::size_t k{ 0 }; k < Z - 1; ++k)
                {
                    const auto numTriangles{ polygonizeCell(i, j, k, isoLevel, triangleAfterPolygonize) };

                    for(unsigned int c{ 0 }; c < numTriangles; ++c)
                    {
                        const auto normal{ triangleAfterPolygonize.at(c).computeNormal() };

                        result.push_back(triangleAfterPolygonize.at(c)[0][0]);
                        result.push_back(triangleAfterPolygonize.at(c)[0][1]);
                        result.push_back(triangleAfterPolygonize.at(c)[0][2]);
                        result.push_back(normal[0]);
                        result.push_back(normal[1]);
                        result.push_back(normal[2]);

                        result.push_back(triangleAfterPolygonize.at(c)[1][0]);
                        result.push_back(triangleAfterPolygonize.at(c)[1][1]);
                        result.push_back(triangleAfterPolygonize.at(c)[1][2]);
                        result.push_back(normal[0]);
                        result.push_back(normal[1]);
                        result.push_back(normal[2]);

                        result.push_back(triangleAfterPolygonize.at(c)[2][0]);
                        result.push_back(triangleAfterPolygonize.at(c)[2][1]);
                        result.push_back(triangleAfterPolygonize.at(c)[2][2]);
                        result.push_back(normal[0]);
                        result.push_back(normal[1]);
                        result.push_back(normal[2]);
                    }
                }
            }
        }

        return result;
    }

private:
    CubeLattice3D<X, Y, Z> m_cubeLattice;
    LatticeData3D<float, X, Y, Z> m_scalarField;

    unsigned int polygonizeCell(
        std::size_t i, std::size_t j, std::size_t k, double isoLevel, std::array<Triangle, 5>& triangleResult // NOLINT
    )
    {
        constexpr auto CORNER_COUNT{ 8 };

        unsigned int cubeIndex{ 0 };
        std::array<glm::ivec3, CORNER_COUNT> corners{};
        for(int idx{ 0 }; idx < CORNER_COUNT; ++idx)
        {
            auto corner{ cellCornerIndexToIJK(idx, i, j, k) };

            if(m_scalarField[corner] < isoLevel)
                cubeIndex |= (1u << idx);

            corners.at(idx) = std::move(corner);
        }

        const auto& edgeMask{ EDGE_TABLE.at(cubeIndex) };
        if(edgeMask == 0)
            return 0;

        std::array<glm::vec3, 12> vertList{}; // NOLINT(readability-magic-numbers)
        for(std::size_t edge{ 0 }; edge < EDGE_CORNERS.size(); ++edge)
        {
            if((edgeMask & (1u << edge)) == 0)
                continue;

            const auto& corner{ EDGE_CORNERS.at(edge) };
            vertList.at(edge) = interpolateVertex(isoLevel, corners.at(corner.x), corners.at(corner.y));
        }

        unsigned int nTriangle{ 0 };
        const auto& tableRow{ TRIANGLE_TABLE.at(cubeIndex) };
        for(std::size_t t{ 0 }; tableRow.at(t) != -1; t += 3)
        {
            auto& triangle{ triangleResult.at(nTriangle++) };

            triangle[0] = vertList.at(tableRow.at(t));
            triangle[1] = vertList.at(tableRow.at(t + 1));
            triangle[2] = vertList.at(tableRow.at(t + 2));
        }

        return nTriangle;
    }

    glm::ivec3 cellCornerIndexToIJK(std::size_t vertexIndex, std::size_t i, std::size_t j, std::size_t k)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        switch(vertexIndex)
        {
        case 0:
            return { i, j, k + 1 };
        case 1:
            return { i + 1, j, k + 1 };
        case 2:
            return { i + 1, j, k };
        case 3:
            return { i, j, k };
        case 4:
            return { i, j + 1, k + 1 };
        case 5:
            return { i + 1, j + 1, k + 1 };
        case 6:
            return { i + 1, j + 1, k };
        case 7:
            return { i, j + 1, k };
        default:
            return {};
        }
        // NOLINTEND(readability-magic-numbers)
    }

    glm::vec3 interpolateVertex(double isoLevel, const glm::ivec3& vert1, const glm::ivec3& vert2)
    {
        const auto& p1{ m_cubeLattice[vert1] };
        const auto& p2{ m_cubeLattice[vert2] };

        const auto val1{ m_scalarField[vert1] };
        const auto val2{ m_scalarField[vert2] };

        double mu{ 0.0 };
        constexpr auto delta{ 0.00001f };

        if(std::abs(isoLevel - val1) < delta)
            return p1;
        if(std::abs(isoLevel - val2) < delta)
            return p2;
        if(std::abs(val1 - val2) < delta)
            return p1;

        mu = (isoLevel - val1) / (val2 - val1);

        return { static_cast<float>(p1[0] + (mu * (p2[0] - p1[0]))),
                 static_cast<float>(p1[1] + (mu * (p2[1] - p1[1]))),
                 static_cast<float>(p1[2] + (mu * (p2[2] - p1[2]))) };
    }
};

} // namespace pen

#endif // !PEN_SRC_ENGINE_TERRAIN_CUBE_LATTICE_SCALAR_FIELD_3D_HPP
