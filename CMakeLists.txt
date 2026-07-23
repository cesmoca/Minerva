cmake_minimum_required(VERSION 3.20)

project(
    Minerva
    VERSION 0.1.0
    DESCRIPTION "Restoration build for the Minerva augmented reality engine"
    LANGUAGES C CXX
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
find_package(SDL REQUIRED)
find_package(OpenGL REQUIRED)
find_package(JPEG REQUIRED)
find_package(PNG REQUIRED)
find_package(SDL_image 1.2 REQUIRED)
find_package(Freetype REQUIRED)
find_package(SDL_ttf 2.0 REQUIRED)

include(FetchContent)
FetchContent_Declare(
    lib3ds
    URL https://deb.debian.org/debian/pool/main/lib3/lib3ds/lib3ds_1.3.0.orig.tar.gz
    URL_HASH SHA512=a315bd0f75cf87d8e285b5a405fe9f033900e23b363cdcf079142dc59edc94e63666e8ab2c0097d939689cd8da0fdcacacaa15f50d0ac3e8a9f5c79b854ab23b
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(lib3ds)

add_library(lib3ds STATIC
    ${lib3ds_SOURCE_DIR}/lib3ds/io.c
    ${lib3ds_SOURCE_DIR}/lib3ds/vector.c
    ${lib3ds_SOURCE_DIR}/lib3ds/matrix.c
    ${lib3ds_SOURCE_DIR}/lib3ds/quat.c
    ${lib3ds_SOURCE_DIR}/lib3ds/tcb.c
    ${lib3ds_SOURCE_DIR}/lib3ds/ease.c
    ${lib3ds_SOURCE_DIR}/lib3ds/chunk.c
    ${lib3ds_SOURCE_DIR}/lib3ds/file.c
    ${lib3ds_SOURCE_DIR}/lib3ds/background.c
    ${lib3ds_SOURCE_DIR}/lib3ds/atmosphere.c
    ${lib3ds_SOURCE_DIR}/lib3ds/shadow.c
    ${lib3ds_SOURCE_DIR}/lib3ds/viewport.c
    ${lib3ds_SOURCE_DIR}/lib3ds/material.c
    ${lib3ds_SOURCE_DIR}/lib3ds/mesh.c
    ${lib3ds_SOURCE_DIR}/lib3ds/camera.c
    ${lib3ds_SOURCE_DIR}/lib3ds/light.c
    ${lib3ds_SOURCE_DIR}/lib3ds/tracks.c
    ${lib3ds_SOURCE_DIR}/lib3ds/node.c
)
target_include_directories(lib3ds SYSTEM PUBLIC ${lib3ds_SOURCE_DIR})
if(MSVC)
    # lib3ds 1.3.0 only distinguished DLL builds in its public headers.
    # Export-marked declarations also allow its unmodified API to link from
    # the static restoration target without __declspec(dllimport).
    target_compile_definitions(lib3ds PUBLIC LIB3DS_EXPORTS)
endif()

# Minerva's public 3D API is coupled to the vendored Bullet 2.78 headers.
# Build only the two Bullet components required by the current bundle.
set(BULLET_PHYSICS_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/bullet-2.78")
set(BULLET_VERSION 2.78)
set(INSTALL_LIBS OFF)
add_subdirectory(lib/bullet-2.78/src/LinearMath EXCLUDE_FROM_ALL)
add_subdirectory(lib/bullet-2.78/src/BulletCollision EXCLUDE_FROM_ALL)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # Bullet 2.78 uses pointer-to-long casts that modern 64-bit GCC rejects.
    # Keep the compatibility relaxation at target level; vendor sources stay
    # unchanged.
    target_compile_options(LinearMath PRIVATE -fpermissive)
    target_compile_options(BulletCollision PRIVATE -fpermissive)
endif()

# This restoration target grows one dependency-ordered subsystem at a time.
# The scene parser, SDL rendering/audio, additional OpenCV modules, ARToolKit,
# and model loaders remain excluded until their prerequisite phases are stable.
add_library(minerva_kernel STATIC
    source/Controllers/InputEventController.cpp
    source/Factories/VideoFactory.cpp
    source/Kernel/EndController.cpp
    source/Kernel/Logger.cpp
    source/Kernel/PathPoint.cpp
    source/Kernel/Parsers/Parser.cpp
    source/Kernel/Parsers/Parser3ds.cpp
    source/Kernel/Parsers/ParserObj.cpp
    source/Kernel/Parsers/ParserOrej.cpp
    source/Kernel/Resources/Resource.cpp
    source/Kernel/Resources/ResourceFile.cpp
    source/Kernel/Resources/ResourceZip.cpp
    source/Kernel/Resources/ResourcesManager.cpp
    source/Kernel/TrackingMethods/TrackingMethod.cpp
    source/Kernel/Parsers/MSLPreprocessor.cpp
    source/Kernel/VideoSource.cpp
    source/MAO/MAO.cpp
    source/MAO/MAOMark.cpp
    source/MAO/MAOMarksGroup.cpp
    source/MAO/MAOPositionator3D.cpp
    source/MAO/MAOProperty.cpp
    source/MAO/MAORenderable2D.cpp
    source/MAO/MAORenderable2DImage.cpp
    source/MAO/MAORenderable2DText.cpp
    source/MAO/MAORenderable3D.cpp
    source/MAO/MAORenderable3DLine.cpp
    source/MAO/MAORenderable3DModel.cpp
    source/MAO/MAORenderable3DPath.cpp
    source/MAO/MAOValue.cpp
    source/MLB/MLB.cpp
    source/MLB/Actuator/MLBActuator.cpp
    source/MLB/Actuator/MLBActuatorAng.cpp
    source/MLB/Actuator/MLBActuatorAnim.cpp
    source/MLB/Actuator/MLBActuatorChangePose.cpp
    source/MLB/Actuator/MLBActuatorDistance.cpp
    source/MLB/Actuator/MLBActuatorPathAddPoint.cpp
    source/MLB/Actuator/MLBActuatorPathRemovePoints.cpp
    source/MLB/Actuator/MLBActuatorProperty.cpp
    source/MLB/Actuator/MLBActuatorQuitApp.cpp
    source/MLB/Actuator/MLBActuatorRandom.cpp
    source/MLB/Actuator/MLBActuatorRelativePose.cpp
    source/MLB/Actuator/MLBActuatorVisibility.cpp
    source/MLB/Controller/MLBController.cpp
    source/MLB/Controller/MLBControllerAND.cpp
    source/MLB/Controller/MLBControllerNAND.cpp
    source/MLB/Controller/MLBControllerNOR.cpp
    source/MLB/Controller/MLBControllerOR.cpp
    source/MLB/Sensor/MLBSensor.cpp
    source/MLB/Sensor/MLBSensorActuator.cpp
    source/MLB/Sensor/MLBSensorAlways.cpp
    source/MLB/Sensor/MLBSensorDelay.cpp
    source/MLB/Sensor/MLBSensorKeyboard.cpp
    source/MLB/Sensor/MLBSensorProperty.cpp
    source/MLB/Sensor/MLBSensorRandom.cpp
    source/MPY/WrapperTypes.cpp
)

target_include_directories(minerva_kernel
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        ${OpenCV_INCLUDE_DIRS}
        ${SDL_INCLUDE_DIRS}
        ${SDL_IMAGE_INCLUDE_DIRS}
        ${SDL_TTF_INCLUDE_DIRS}
)

target_include_directories(minerva_kernel SYSTEM PUBLIC
    "${BULLET_PHYSICS_SOURCE_DIR}/src"
)

target_link_libraries(minerva_kernel
    PUBLIC
        Boost::filesystem
        Boost::python
        libzip::zip
        ${OpenCV_LIBS}
        ${SDL_LIBRARIES}
        ${SDL_IMAGE_LIBRARIES}
        ${SDL_TTF_LIBRARIES}
        JPEG::JPEG
        PNG::PNG
        Freetype::Freetype
        OpenGL::GL
        lib3ds
        BulletCollision
        LinearMath
)

if(WIN32)
    # The legacy code tests WIN32 rather than the compiler-provided _WIN32.
    target_compile_definitions(minerva_kernel PUBLIC WIN32)
    # SDL 1.2's static Windows build leaves these platform libraries to users.
    target_link_libraries(minerva_kernel PUBLIC winmm dxguid)
endif()

if(MSVC)
    target_compile_options(minerva_kernel PRIVATE /W4 /permissive- /EHsc)
else()
    target_compile_options(minerva_kernel PRIVATE -Wall -Wextra -Wpedantic)
    # Public Bullet 2.78 headers contain the same 64-bit casts as its sources.
    target_compile_options(minerva_kernel PUBLIC -fpermissive)
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
        tests/Controllers/InputEventControllerTests.cpp
        tests/Factories/VideoFactoryTests.cpp
        tests/Kernel/EndControllerTests.cpp
        tests/Kernel/LoggerTests.cpp
        tests/Kernel/PathPointTests.cpp
        tests/Kernel/Parsers/ParserTests.cpp
        tests/Kernel/Parsers/Parser3dsTests.cpp
        tests/Kernel/Parsers/MSLPreprocessorTests.cpp
        tests/Kernel/Resources/ResourceTests.cpp
        tests/Kernel/Resources/ResourceFileTests.cpp
        tests/Kernel/Resources/ResourceZipTests.cpp
        tests/Kernel/Resources/ResourcesManagerTests.cpp
        tests/Kernel/TrackingMethods/TrackingMethodTests.cpp
        tests/Kernel/VideoSourceTests.cpp
        tests/MAO/MAOTests.cpp
        tests/MAO/MAOMarkTests.cpp
        tests/MAO/MAOMarksGroupTests.cpp
        tests/MAO/MAOPositionator3DTests.cpp
        tests/MAO/MAOPropertyTests.cpp
        tests/MAO/MAORenderable2DTests.cpp
        tests/MAO/MAORenderable2DTextTests.cpp
        tests/MAO/MAORenderable3DTests.cpp
        tests/MAO/MAOValueTests.cpp
        tests/MLB/MLBTests.cpp
        tests/MLB/Actuator/MLBActuatorTests.cpp
        tests/MLB/Actuator/MLBActuator3DTests.cpp
        tests/MLB/Actuator/MLBActuatorVisibilityTests.cpp
        tests/MLB/Controller/MLBControllerTests.cpp
        tests/MLB/Sensor/MLBSensorTests.cpp
        tests/MLB/Sensor/MLBSensorKeyboardTests.cpp
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
