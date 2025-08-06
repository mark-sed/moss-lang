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
    # FIXME: Don't use hardcoded paths
    set(LIBFFI_INCLUDE_DIRS "C:/dev/vcpkg/packages/libffi_x64-windows/include")
    set(LIBFFI_LIBRARIES "C:/dev/vcpkg/packages/libffi_x64-windows/lib/ffi.lib")
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBFFI REQUIRED libffi)
endif()

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
    stdlib/cffi.cpp
    stdlib/subprocess.cpp
    stdlib/sys.cpp
    stdlib/time.cpp
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

# Function to copy file from stdlib into lib directory
function(copy_lib_file filename)
  set(OUTPUT_FILE ${LIBMS_PATH}/${filename})

  add_custom_command(
    OUTPUT ${OUTPUT_FILE}
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/stdlib/${filename}
            ${LIBMS_PATH}/${filename}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/stdlib/${filename}
    COMMENT "Copying ${filename} to ${LIBMS_PATH}"
    VERBATIM
  )

  add_custom_target(copy_${filename} ALL DEPENDS ${OUTPUT_FILE})
endfunction()

# Function to compile moss module into msb
function(moss_compile NAME)
    set(INPUT_MS "${CMAKE_CURRENT_SOURCE_DIR}/stdlib/${NAME}.ms")
    set(OUTPUT_MSB "${LIBMS_PATH}/${NAME}.msb")

    add_custom_command(
        OUTPUT ${OUTPUT_MSB}
        COMMAND ${PROJECT_NAME} -W all --compile-only -o ${OUTPUT_MSB} ${INPUT_MS}
        DEPENDS ${INPUT_MS}
        COMMENT "Building ${NAME}"
        VERBATIM
    )

    add_custom_target(build_${NAME} ALL DEPENDS ${OUTPUT_MSB})
endfunction()

# Standard moss library modules and files.
# Any module/file added here has to be added to libms target bellow (as `build_name`/`copy_name`).
moss_compile(libms)
moss_compile(cffi)
moss_compile(subprocess)
moss_compile(sys)
moss_compile(time)
moss_compile(md_parser)
moss_compile(html_parser)

copy_lib_file("mossy.css")

add_custom_target(
    libms
    DEPENDS build_libms
    DEPENDS build_cffi
    DEPENDS build_subprocess
    DEPENDS build_sys
    DEPENDS build_time
    DEPENDS build_md_parser
    DEPENDS build_html_parser
    DEPENDS copy_mossy.css
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

# Include libffi headers
target_include_directories(${PROJECT_NAME} PRIVATE ${LIBFFI_INCLUDE_DIRS})

target_compile_definitions(${PROJECT_NAME} PRIVATE ${LIBFFI_CFLAGS_OTHER})

if (WIN32)
    target_link_libraries(${PROJECT_NAME} msvcrt ${LIBFFI_LIBRARIES})
else()
    target_link_libraries(${PROJECT_NAME} m ${LIBFFI_LIBRARIES})
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
        ${LIBFFI_LIBRARIES}
    )

    # GTest include
    include(GoogleTest)
    gtest_discover_tests(testsmoss)
    target_include_directories(testsmoss PRIVATE ${LIBFFI_INCLUDE_DIRS})

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