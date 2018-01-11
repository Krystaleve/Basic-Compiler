cmake_minimum_required(VERSION 3.9)
project(YetAnotherCompiler CXX C ASM)

set(CMAKE_C_STANDARD 90)
set(CMAKE_CXX_STANDARD 11)

find_package(BISON REQUIRED)
set(BisonOutput ${CMAKE_SOURCE_DIR}/src/syntax/syntax.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unused-parameter -Wno-unused-function")

add_custom_command(
        OUTPUT ${BisonOutput}
        COMMAND ${BISON_EXECUTABLE}
        --defines=${CMAKE_SOURCE_DIR}/src/syntax/syntax.h
        --output=${BisonOutput}
        ${CMAKE_SOURCE_DIR}/src/syntax/syntax.y
        MAIN_DEPENDENCY ${CMAKE_SOURCE_DIR}/src/syntax/syntax.y
        COMMENT "Generating syntax.cpp"
)

find_package(FLEX REQUIRED)
set(FlexOutput ${CMAKE_SOURCE_DIR}/src/syntax/lexical.cpp)
add_custom_command(
        OUTPUT ${FlexOutput}
        COMMAND ${FLEX_EXECUTABLE}
        --outfile=${FlexOutput}
        ${CMAKE_SOURCE_DIR}/src/syntax/lexical.l
        MAIN_DEPENDENCY ${CMAKE_SOURCE_DIR}/src/syntax/lexical.l
        COMMENT "Generating lexical.cpp"
)

find_package(LLVM REQUIRED CONFIG)

add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})

add_executable(yac
        ${FlexOutput}
        ${BisonOutput}
        src/ast/ast.h
        src/ast/ast.cpp
        src/ast/declaration.h
        src/ast/declaration.cpp
        src/ast/expression.h
        src/ast/expression.cpp
        src/ast/statement.h
        src/ast/statement.cpp
        src/ast/context.h
        src/ast/context.cpp
        src/ast/type.h
        src/ast/type.cpp
        src/main.cpp)

llvm_map_components_to_libnames(llvm_libs core executionengine x86asmparser x86asmprinter x86codegen mcjit)
target_link_libraries(yac ${llvm_libs})

foreach(Test tests palindromic kmp calc)
    add_executable(${Test}-cc tests/${Test}.c)
    add_custom_command(
            OUTPUT ${CMAKE_SOURCE_DIR}/tests/${Test}-yac.c
            COMMAND ${CMAKE_C_COMPILER} ${Test}.c -E -o ${Test}-yac.c ${CMAKE_C_FLAGS}
            DEPENDS ${CMAKE_SOURCE_DIR}/tests/${Test}.c
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/src/tests
            COMMENT "Generating ${Test}-yac.c"
    )
    add_custom_command(
            OUTPUT ${CMAKE_SOURCE_DIR}/tests/${Test}-yac.ll
            COMMAND $<TARGET_FILE:yac> -o ${Test}-yac.ll ${Test}-yac.c
            DEPENDS ${CMAKE_SOURCE_DIR}/tests/${Test}-yac.c
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/src/tests
            COMMENT "Generating ${Test}-yac.ll"
    )
    add_custom_command(
            OUTPUT ${CMAKE_SOURCE_DIR}/tests/${Test}-yac.s
            COMMAND llc -relocation-model=pic ${Test}-yac.ll
            DEPENDS ${CMAKE_SOURCE_DIR}/tests/${Test}-yac.ll
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tests
            COMMENT "Generating ${Test}-yac.s"
    )
    add_executable(${Test}-yac ${CMAKE_SOURCE_DIR}/tests/${Test}-yac.s)
endforeach(Test)
