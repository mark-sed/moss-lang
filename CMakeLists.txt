cmake_minimum_required(VERSION 3.8...3.31)

# Setup project, languages and standards
project(moss)
set(CMAKE_CXX_STANDARD 17)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-O1 -g -DDEVELOPER")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

if (MSVC)
    #add_compile_options(/W4 /WX)
    add_compile_options(/MD)
    set(LIBMS_PATH "$ENV{LOCALAPPDATA}\\moss")
    set(MOSS_PATH "$ENV{LOCALAPPDATA}\\moss\\bin")
    file(MAKE_DIRECTORY ${LIBMS_PATH})
    file(MAKE_DIRECTORY ${MOSS_PATH})
else()
    add_compile_options(-Wall -Wextra -pedantic -Wno-mismatched-new-delete)
    set(LIBMS_PATH "/lib/moss")
    set(MOSS_PATH "/usr/bin/moss")
    file(MAKE_DIRECTORY ${LIBMS_PATH})
endif()

# Includes for headers
include_directories(.)
include_directories(frontend)
include_directories(midend)
include_directories(stdlib)
include_directories(bytecode)
include_directories(vm)
include_directories(interface)

# Source files excluding main
set(SOURCES 
    frontend/parser.cpp
    frontend/scanner.cpp
    frontend/source.cpp
    midend/ir.cpp
    midend/ir_pipeline.cpp
    midend/ir_visitor.cpp
    midend/method_analyzer.cpp
    stdlib/mslib.cpp
    stdlib/mslib_file.cpp
    stdlib/mslib_list.cpp
    stdlib/mslib_string.cpp
    stdlib/subprocess.cpp
    interface/commons.cpp
    interface/clopts.cpp
    interface/diagnostics.cpp
    interface/errors.cpp
    interface/logging.cpp
    interface/repl.cpp
    interface/utils.cpp
    bytecode/bytecode.cpp
    bytecode/bytecodegen.cpp
    bytecode/bytecode_reader.cpp
    bytecode/bytecode_writer.cpp
    bytecode/opcode.cpp
    vm/gc.cpp
    vm/interpreter.cpp
    vm/memory.cpp
    vm/values.cpp
    stdlib/builtins.cpp # builtins has to be after values
)

# Targets
# moss target
add_executable(
    ${PROJECT_NAME}
    moss.cpp
    ${SOURCES}
)

# Function to compile moss module into msb
function(moss_compile NAME)
    set(INPUT_MS "${CMAKE_CURRENT_SOURCE_DIR}/stdlib/${NAME}.ms")
    set(OUTPUT_MSB "${LIBMS_PATH}/${NAME}.msb")

    add_custom_command(
        OUTPUT ${OUTPUT_MSB}
        COMMAND ${PROJECT_NAME} -W all --output-only -o ${OUTPUT_MSB} ${INPUT_MS}
        DEPENDS ${INPUT_MS}
        COMMENT "Building ${NAME}"
        VERBATIM
    )

    add_custom_target(build_${NAME} ALL DEPENDS ${OUTPUT_MSB})
endfunction()

# Standard moss library modules
# Any module added here has to be added to libms target bellow (as `build_name`)
moss_compile(libms)
moss_compile(subprocess)

add_custom_target(
    libms
    DEPENDS build_libms
    DEPENDS build_subprocess
)

# Making moss a command

if (MSVC)
    add_custom_command(
        OUTPUT binmoss    
        #message(STATUS "To have moss runnable from your CMD add ${CMAKE_CURRENT_BINARY_DIR}/Debug to your PATH")
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/moss.exe
                ${MOSS_PATH}
    )
else()
    add_custom_command(
        OUTPUT binmoss
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_BINARY_DIR}/moss
                ${MOSS_PATH}
    )
endif()

add_custom_target(
    installation
    DEPENDS binmoss
)


if (WIN32)
    target_link_libraries(${PROJECT_NAME} msvcrt)
else()
    target_link_libraries(${PROJECT_NAME} m)
endif()

# testsmoss target
if(CMAKE_BUILD_TYPE STREQUAL "Debug")

    # Test source files
    set(TEST_SRCS
        tests/unittests/testing_utils.cpp
        tests/unittests/test_bytecode_rw.cpp
        tests/unittests/test_bytecode.cpp
        tests/unittests/test_clopts.cpp
        tests/unittests/test_interpreter.cpp
        tests/unittests/test_ir.cpp
        tests/unittests/test_memory.cpp
        tests/unittests/test_method_analysis.cpp
        tests/unittests/test_parser_examples.cpp
        tests/unittests/test_parser.cpp
        tests/unittests/test_scanner.cpp
        )

    # Enable gtest
    enable_testing()
    add_subdirectory(tests/googletest)

    add_executable(
        testsmoss
        ${TEST_SRCS}
        ${SOURCES}
    )

    target_link_libraries(
        testsmoss
        gtest_main
    )

    # GTest include
    include(GoogleTest)
    gtest_discover_tests(testsmoss)

    # Docs build
    # docs target
    find_package(Doxygen REQUIRED dot)
    if(DOXYGEN_FOUND)
        set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile)
        set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

        add_custom_target( docs ALL
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM )
    else (DOXYGEN_FOUND)
        message("Doxygen need to be installed to generate the doxygen documentation")
    endif (DOXYGEN_FOUND)

endif(CMAKE_BUILD_TYPE STREQUAL "Debug")