cmake_minimum_required(VERSION 3.20)

project(SDL_ttf VERSION 2.0.11 LANGUAGES C)

find_path(SDL_INCLUDE_DIR SDL.h PATH_SUFFIXES SDL REQUIRED)
find_library(SDL_LIBRARY NAMES SDL REQUIRED)
find_package(Freetype REQUIRED)

add_library(SDL_ttf SDL_ttf.c)

target_compile_definitions(SDL_ttf PRIVATE _CRT_SECURE_NO_WARNINGS)

target_include_directories(SDL_ttf
    PRIVATE
        "${SDL_INCLUDE_DIR}"
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include/SDL>
)

target_link_libraries(SDL_ttf PRIVATE
    "${SDL_LIBRARY}"
    Freetype::Freetype
)

install(TARGETS SDL_ttf
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
install(FILES SDL_ttf.h DESTINATION include/SDL)
