#ifndef PEN_SRC_APP_APPLICATION_HPP
#define PEN_SRC_APP_APPLICATION_HPP

#include "opengl/Shader.hpp"
#include "rendering/GlfwContext.hpp"
#include "rendering/Window.hpp"
#include "terrain/CubeLatticeScalarField3D.hpp"
#include "terrain/LatticeData3D.hpp"
#include "terrain/Sphere.hpp"

#include <glm/glm.hpp>

#include <memory>

namespace
{

constexpr std::size_t LATTICE_X{ 30 };
constexpr std::size_t LATTICE_Y{ 30 };
constexpr std::size_t LATTICE_Z{ 30 };

} // namespace

namespace pen::app
{

/// \brief The \ref Application is the orchestrator of the program flow.
///
/// \author Felix Hommel
/// \date 6/26/2026
class Application
{
public:
    Application();
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    Application(Application&&) = delete;
    Application& operator=(Application&&) = delete;

    /// \brief Start the application.
    void start();

private:
    using LatticeData = LatticeData3D<float, ::LATTICE_X, ::LATTICE_Y, ::LATTICE_Z>;
    using ScalarField = CubeLatticeScalarField3D<::LATTICE_X, ::LATTICE_Y, ::LATTICE_Z>;

    std::unique_ptr<GlfwContext> glfw;
    std::unique_ptr<Window> window;
    std::unique_ptr<Shader> marchingCubes;
    std::unique_ptr<Shader> lighting;
    std::unique_ptr<Sphere> lightSphere;
    std::unique_ptr<LatticeData> scalarField;
    std::unique_ptr<ScalarField> grid;

    unsigned int vao{ 0 };
    std::size_t numberOfVerticesToDraw{ 0 };

    void update();
    void render() const;

    void assignScalarField(const glm::vec3& center = glm::vec3(0.f));
    void bufferGridDataGL(double isoLevel);
    void drawGrid() const;
};

} // namespace pen::app

#endif // !PEN_SRC_APP_APPLICATION_HPP
