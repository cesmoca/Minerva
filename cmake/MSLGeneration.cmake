if(MINERVA_BISONPP_EXECUTABLE AND MINERVA_FLEXPP_EXECUTABLE)
    set(generator_mode)
    if(MINERVA_MSL_OUTPUT_MISSING)
        set(generator_mode ALL)
    endif()
    add_custom_target(minerva_generate_msl ${generator_mode}
        COMMAND ${CMAKE_COMMAND}
            -DMINERVA_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
            -DMINERVA_BISONPP_EXECUTABLE=${MINERVA_BISONPP_EXECUTABLE}
            -DMINERVA_FLEXPP_EXECUTABLE=${MINERVA_FLEXPP_EXECUTABLE}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/GenerateMSL.cmake
        COMMENT "Generating the MSL parser and scanner"
        VERBATIM
    )
else()
    add_custom_target(minerva_generate_msl
        COMMAND ${CMAKE_COMMAND} -E echo See_BUILDING.md_for_MSL_generator_setup.
        COMMAND ${CMAKE_COMMAND} -E false
    )
    if(MINERVA_MSL_OUTPUT_MISSING)
        message(WARNING Generated_MSL_files_are_missing._See_BUILDING.md.)
    endif()
endif()
