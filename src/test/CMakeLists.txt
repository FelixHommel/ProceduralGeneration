set(NAME "ProceduralGenerationTests")

include(${PROJECT_SOURCE_DIR}/cmake/StaticAnalyzers.cmake)

add_executable(${NAME} ./testMain.cpp ./opengl/ShaderTest.cpp)

target_sources(
    ${NAME}
    PRIVATE
        FILE_SET HEADERS
            FILES
                ./testUtility/OpenGLTestFixture.hpp
                ./testUtility/RandomNumberGenerator.hpp
)

target_compile_features(${NAME} PRIVATE cxx_std_23)
target_link_libraries(
    ${NAME}
    PRIVATE project_warnings GTest::gtest GTest::gmock ProceduralGenerationLib
)

include(GoogleTest)
gtest_discover_tests(${NAME})
