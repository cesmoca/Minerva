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

set(MINERVA_MSL_OUTPUTS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Kernel/Parsers/MSLParser.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/Kernel/Parsers/MSLParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/Kernel/Parsers/MSLScanner.cpp
)
set(MINERVA_MSL_OUTPUT_MISSING OFF)
foreach(generated_file ${MINERVA_MSL_OUTPUTS})
    if(NOT EXISTS ${generated_file})
        set(MINERVA_MSL_OUTPUT_MISSING ON)
    endif()
endforeach()

find_program(MINERVA_BISONPP_EXECUTABLE
    NAMES bison++
    HINTS C:/tmp/minerva-parser-tools/install/bin
)
find_program(MINERVA_FLEXPP_EXECUTABLE
    NAMES flex++
    HINTS C:/msys64/usr/bin
)

include(cmake/MSLGeneration.cmake)

# This first restoration target intentionally contains only code that depends
# on the C++ standard library. The scene parser, Python, SDL/OpenGL, OpenCV,
# ARToolKit, Bullet, audio, resource packaging, and model loaders are excluded
# until the build baseline is stable.
add_library(minerva_core STATIC
    source/Kernel/EndController.cpp
    source/Kernel/Logger.cpp
    source/Kernel/PathPoint.cpp
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

option(BUILD_TESTING "Build Minerva tests" ON)

if(BUILD_TESTING)
    enable_testing()
    add_test(NAME minerva.smoke COMMAND minerva_smoke)

    add_executable(minerva_kernel_tests
        tests/minerva_kernel_tests.cpp
    )
    target_link_libraries(minerva_kernel_tests PRIVATE minerva_core)

    if(MSVC)
        target_compile_options(minerva_kernel_tests PRIVATE /W4 /permissive- /EHsc)
    else()
        target_compile_options(minerva_kernel_tests PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    set(MINERVA_KERNEL_TEST_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/tests/kernel)
    file(MAKE_DIRECTORY ${MINERVA_KERNEL_TEST_DIRECTORY})
    add_test(NAME minerva.kernel.unit COMMAND minerva_kernel_tests)
    set_tests_properties(minerva.kernel.unit PROPERTIES
        WORKING_DIRECTORY ${MINERVA_KERNEL_TEST_DIRECTORY}
    )

    add_custom_target(minerva_tests
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -C $<CONFIG>
        DEPENDS minerva_smoke minerva_kernel_tests
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        USES_TERMINAL
    )
endif()
