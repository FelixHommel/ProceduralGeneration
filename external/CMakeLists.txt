set(FASTNOISE2_TOOLS OFF CACHE BOOL "Build \"Node Editor\" executable" FORCE)
set(FASTNOISE2_TESTS OFF CACHE BOOL "Build tests" FORCE)
set(FASTNOISE2_UTILITY OFF CACHE BOOL "Build utility tools" FORCE)
set(FASTNOISE2_STRICT_FP
    OFF
    CACHE BOOL
    "Enable strict floating point calculations to ensure output from different SIMD feature sets match EXACTLY"
    FORCE
)
add_subdirectory(FastNoise2 SYSTEM)
