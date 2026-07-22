vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO libsdl-org/SDL_ttf
    REF 831efa364afb87c0c93e7635153e6df9be3c72b4
    SHA512 d6294a064a6efdd5ceedd15efe79717925a476c361b785511114d9c3652bec20d604057954cc8601c8ba87c45907d69c7691ed5010068254f4ac2179070d5b32
    HEAD_REF main
)

file(COPY "${CURRENT_PORT_DIR}/CMakeLists.txt" DESTINATION "${SOURCE_PATH}")

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
)
vcpkg_cmake_install()
vcpkg_copy_pdbs()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/COPYING")
