set(minerva_stdout_file "${MINERVA_WORKING_DIRECTORY}/stdout.txt")
set(minerva_stderr_file "${MINERVA_WORKING_DIRECTORY}/stderr.txt")
get_filename_component(minerva_executable_directory
    "${MINERVA_EXECUTABLE}" DIRECTORY
)
set(minerva_executable_stdout_file
    "${minerva_executable_directory}/stdout.txt"
)
set(minerva_executable_stderr_file
    "${minerva_executable_directory}/stderr.txt"
)
file(REMOVE
    "${minerva_stdout_file}"
    "${minerva_stderr_file}"
    "${minerva_executable_stdout_file}"
    "${minerva_executable_stderr_file}"
)

execute_process(
    COMMAND "${MINERVA_EXECUTABLE}"
    WORKING_DIRECTORY "${MINERVA_WORKING_DIRECTORY}"
    RESULT_VARIABLE minerva_result
    OUTPUT_VARIABLE minerva_output
    ERROR_VARIABLE minerva_error
)

if(minerva_result EQUAL 0)
    message(FATAL_ERROR "minerva without arguments unexpectedly succeeded")
endif()

foreach(redirected_file IN ITEMS
        "${minerva_stdout_file}"
        "${minerva_stderr_file}"
        "${minerva_executable_stdout_file}"
        "${minerva_executable_stderr_file}"
)
    if(EXISTS "${redirected_file}")
        file(READ "${redirected_file}" redirected_output)
        string(APPEND minerva_output "${redirected_output}")
    endif()
endforeach()

string(FIND "${minerva_output}${minerva_error}" "Usage:" usage_position)
if(usage_position EQUAL -1)
    message(FATAL_ERROR
        "minerva without arguments did not print its usage message:\n"
        "${minerva_output}${minerva_error}"
    )
endif()
