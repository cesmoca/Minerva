if(NOT DEFINED MINERVA_SOURCE_DIR)
    message(FATAL_ERROR MINERVA_SOURCE_DIR_is_required)
endif()
if(NOT DEFINED MINERVA_BISONPP_EXECUTABLE)
    message(FATAL_ERROR MINERVA_BISONPP_EXECUTABLE_is_required)
endif()
if(NOT DEFINED MINERVA_FLEXPP_EXECUTABLE)
    message(FATAL_ERROR MINERVA_FLEXPP_EXECUTABLE_is_required)
endif()

if(WIN32 AND EXISTS C:/msys64/ucrt64/bin)
    set(ENV{PATH} "C:/msys64/ucrt64/bin;$ENV{PATH}")
endif()

execute_process(
    COMMAND ${MINERVA_BISONPP_EXECUTABLE}
        -d
        -hinclude/Kernel/Parsers/MSLParser.h
        -o source/Kernel/Parsers/MSLParser.cpp
        source/Kernel/Parsers/MSLParser.y
    WORKING_DIRECTORY ${MINERVA_SOURCE_DIR}
    RESULT_VARIABLE bisonpp_result
)
if(NOT bisonpp_result EQUAL 0)
    message(FATAL_ERROR Bison++_failed_with_exit_code_${bisonpp_result})
endif()

execute_process(
    COMMAND ${MINERVA_FLEXPP_EXECUTABLE}
        -d
        -osource/Kernel/Parsers/MSLScanner.cpp
        source/Kernel/Parsers/MSLScanner.l
    WORKING_DIRECTORY ${MINERVA_SOURCE_DIR}
    RESULT_VARIABLE flexpp_result
)
if(NOT flexpp_result EQUAL 0)
    message(FATAL_ERROR Flex++_failed_with_exit_code_${flexpp_result})
endif()
