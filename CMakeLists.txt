cmake_minimum_required(VERSION 3.20)

project(
    Minerva
    VERSION 0.1.0
    DESCRIPTION "Restoration build for the Minerva augmented reality engine"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# This first restoration target intentionally contains only code that depends
# on the C++ standard library. The scene parser, Python, SDL/OpenGL, OpenCV,
# ARToolKit, Bullet, audio, resource packaging, and model loaders are excluded
# until the build baseline is stable.
add_library(minerva_core STATIC
    source/Kernel/EndController.cpp
    source/Kernel/Logger.cpp
    source/Kernel/Parsers/MSLPreprocessor.cpp
)

target_include_directories(minerva_core
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

if(WIN32)
    # The legacy code tests WIN32 rather than the compiler-provided _WIN32.
    target_compile_definitions(minerva_core PUBLIC WIN32)
endif()

if(MSVC)
    target_compile_options(minerva_core PRIVATE /W4 /permissive- /EHsc)
else()
    target_compile_options(minerva_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(minerva_smoke
    source/minerva_smoke.cpp
)

target_link_libraries(minerva_smoke PRIVATE minerva_core)

if(MSVC)
    target_compile_options(minerva_smoke PRIVATE /W4 /permissive- /EHsc)
else()
    target_compile_options(minerva_smoke PRIVATE -Wall -Wextra -Wpedantic)
endif()

include(CTest)

if(BUILD_TESTING)
    add_test(NAME minerva.core.smoke COMMAND minerva_smoke)
endif()
