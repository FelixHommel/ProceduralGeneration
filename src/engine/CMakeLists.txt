set(NAME "ProceduralGenerationLib")
include(${PROJECT_SOURCE_DIR}/cmake/StaticAnalyzers.cmake)

add_library(
    ${NAME}
    ./opengl/Shader.cpp
    ./rendering/GlfwContext.cpp
    ./rendering/Window.cpp
    ./terrain/Sphere.cpp
)

target_sources(
    ${NAME}
    PRIVATE
        FILE_SET HEADERS
            FILES
                ./opengl/Shader.hpp
                ./rendering/GlfwContext.hpp
                ./rendering/Window.hpp
                ./terrain/CubeLattice3D.hpp
                ./terrain/CubeLatticeScalarField3D.hpp
                ./terrain/LatticeData3D.hpp
                ./terrain/MarchingCubesData.hpp
                ./terrain/Sphere.hpp
                ./utility/Assert.hpp
)

target_compile_features(${NAME} PRIVATE cxx_std_23)

if(MSVC)
    target_compile_options(
        ${NAME}
        PRIVATE /external:W0 /external:anglebrackets /external:templates-
    )
endif()

target_link_libraries(
    ${NAME}
    PUBLIC glm::glm
    PRIVATE
        Glad2CMake::glad2cmake
        glfw
        OpenGL::GL
        project_warnings
        spdlog::spdlog
)
target_include_directories(${NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
