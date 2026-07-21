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

find_package(Boost 1.70 CONFIG REQUIRED COMPONENTS filesystem python)
set(ZLIB_USE_STATIC_LIBS ON)
find_package(libzip CONFIG REQUIRED)
find_package(OpenCV 4 REQUIRED COMPONENTS core videoio)

# This restoration target grows one dependency-ordered subsystem at a time.
# The scene parser, Python, SDL/OpenGL, additional OpenCV modules, ARToolKit,
# Bullet, audio, and model loaders remain excluded until their prerequisite
# phases are stable.
add_library(minerva_kernel STATIC
    source/Factories/VideoFactory.cpp
    source/Kernel/EndController.cpp
    source/Kernel/Logger.cpp
    source/Kernel/PathPoint.cpp
    source/Kernel/Resources/Resource.cpp
    source/Kernel/Resources/ResourceFile.cpp
    source/Kernel/Resources/ResourceZip.cpp
    source/Kernel/Resources/ResourcesManager.cpp
    source/Kernel/TrackingMethods/TrackingMethod.cpp
    source/Kernel/Parsers/MSLPreprocessor.cpp
    source/Kernel/VideoSource.cpp
    source/MAO/MAO.cpp
    source/MAO/MAOMark.cpp
    source/MAO/MAOPositionator3D.cpp
    source/MAO/MAOProperty.cpp
    source/MAO/MAOValue.cpp
    source/MPY/WrapperTypes.cpp
)

target_include_directories(minerva_kernel
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(minerva_kernel
    PUBLIC
        Boost::filesystem
        Boost::python
        libzip::zip
        ${OpenCV_LIBS}
)

if(WIN32)
    # The legacy code tests WIN32 rather than the compiler-provided _WIN32.
    target_compile_definitions(minerva_kernel PUBLIC WIN32)
endif()

if(MSVC)
    target_compile_options(minerva_kernel PRIVATE /W4 /permissive- /EHsc)
else()
    target_compile_options(minerva_kernel PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(minerva_smoke
    source/minerva_smoke.cpp
)

target_link_libraries(minerva_smoke PRIVATE minerva_kernel)

if(MSVC)
    target_compile_options(minerva_smoke PRIVATE /W4 /permissive- /EHsc)
else()
    target_compile_options(minerva_smoke PRIVATE -Wall -Wextra -Wpedantic)
endif()

option(BUILD_TESTING "Build Minerva tests" ON)

if(BUILD_TESTING)
    enable_testing()
    add_test(NAME minerva.smoke COMMAND minerva_smoke)

    find_package(GTest CONFIG QUIET)
    if(NOT GTest_FOUND)
        include(FetchContent)
        set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
        set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG v1.17.0
            GIT_SHALLOW TRUE
        )
        FetchContent_MakeAvailable(googletest)
    endif()

    add_executable(minerva_kernel_tests
        tests/Factories/VideoFactoryTests.cpp
        tests/Kernel/EndControllerTests.cpp
        tests/Kernel/LoggerTests.cpp
        tests/Kernel/PathPointTests.cpp
        tests/Kernel/Parsers/MSLPreprocessorTests.cpp
        tests/Kernel/Resources/ResourceTests.cpp
        tests/Kernel/Resources/ResourceFileTests.cpp
        tests/Kernel/Resources/ResourceZipTests.cpp
        tests/Kernel/Resources/ResourcesManagerTests.cpp
        tests/Kernel/TrackingMethods/TrackingMethodTests.cpp
        tests/Kernel/VideoSourceTests.cpp
        tests/MAO/MAOTests.cpp
        tests/MAO/MAOMarkTests.cpp
        tests/MAO/MAOPositionator3DTests.cpp
        tests/MAO/MAOPropertyTests.cpp
        tests/MAO/MAOValueTests.cpp
        tests/MPY/WrapperTypesTests.cpp
    )
    target_link_libraries(minerva_kernel_tests PRIVATE
        minerva_kernel
        GTest::gtest_main
    )

    if(MSVC)
        target_compile_options(minerva_kernel_tests PRIVATE /W4 /permissive- /EHsc)
    else()
        target_compile_options(minerva_kernel_tests PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    set(MINERVA_KERNEL_TEST_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/tests/kernel)
    file(MAKE_DIRECTORY ${MINERVA_KERNEL_TEST_DIRECTORY})
    include(GoogleTest)
    gtest_discover_tests(minerva_kernel_tests
        WORKING_DIRECTORY ${MINERVA_KERNEL_TEST_DIRECTORY}
        DISCOVERY_MODE PRE_TEST
    )

    add_custom_target(minerva_tests
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -C $<CONFIG>
        DEPENDS minerva_smoke minerva_kernel_tests
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        USES_TERMINAL
    )
endif()
