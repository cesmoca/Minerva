vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO libsdl-org/SDL_image
    REF 220be3fd43a85921138072ed847b7e4bc5ad163e
    SHA512 dbdf0ecd65bac7f889252ab7c42f960e92123046507bf08d2d6edc19c7973431ba75843321e44035ce401fc4b4d143293b60d67d45e2c0ef616d4f99e07a20dc
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
