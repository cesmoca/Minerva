cmake_minimum_required(VERSION 3.20)

project(SDL_image VERSION 1.2.12 LANGUAGES C)

find_path(SDL_INCLUDE_DIR SDL.h PATH_SUFFIXES SDL REQUIRED)
find_library(SDL_LIBRARY NAMES SDL REQUIRED)
find_package(JPEG REQUIRED)
find_package(PNG REQUIRED)

add_library(SDL_image
    IMG.c
    IMG_bmp.c
    IMG_gif.c
    IMG_jpg.c
    IMG_lbm.c
    IMG_pcx.c
    IMG_png.c
    IMG_pnm.c
    IMG_tga.c
    IMG_tif.c
    IMG_webp.c
    IMG_xcf.c
    IMG_xpm.c
    IMG_xv.c
)

target_compile_definitions(SDL_image PRIVATE
    LOAD_BMP
    LOAD_GIF
    LOAD_JPG
    LOAD_LBM
    LOAD_PCX
    LOAD_PNG
    LOAD_PNM
    LOAD_TGA
    LOAD_XPM
    LOAD_XV
    _CRT_SECURE_NO_WARNINGS
)

target_include_directories(SDL_image
    PRIVATE
        "${SDL_INCLUDE_DIR}"
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include/SDL>
)

target_link_libraries(SDL_image PRIVATE
    "${SDL_LIBRARY}"
    JPEG::JPEG
    PNG::PNG
)

install(TARGETS SDL_image
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
install(FILES SDL_image.h DESTINATION include/SDL)
