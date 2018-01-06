cmake_minimum_required(VERSION 3.9)
project(Basic_Compiler)

set(CMAKE_CXX_STANDARD 11)

find_package(BISON REQUIRED)
set(BisonOutput ${CMAKE_SOURCE_DIR}/src/syntax/syntax.cpp)
IF(BISON_FOUND)
    ADD_CUSTOM_COMMAND(
            OUTPUT ${BisonOutput}
            COMMAND ${BISON_EXECUTABLE}
            --defines=${CMAKE_SOURCE_DIR}/src/syntax/syntax.h
            --output=${BisonOutput}
            ${CMAKE_SOURCE_DIR}/src/syntax/syntax.y
            MAIN_DEPENDENCY ${CMAKE_SOURCE_DIR}/src/syntax/syntax.y
            COMMENT "Generating syntax.cpp"
    )
ENDIF()

find_package(FLEX REQUIRED)
set(FlexOutput ${CMAKE_SOURCE_DIR}/src/syntax/lexical.cpp)
IF(FLEX_FOUND)
    add_custom_command(
            OUTPUT ${FlexOutput}
            COMMAND ${FLEX_EXECUTABLE}
            --outfile=${FlexOutput}
            ${CMAKE_SOURCE_DIR}/src/syntax/lexical.l
            MAIN_DEPENDENCY ${CMAKE_SOURCE_DIR}/src/syntax/lexical.l
            COMMENT "Generating lexical.cpp"
    )
ENDIF()

find_package(LLVM REQUIRED CONFIG)

add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})

add_executable(yac
        ${FlexOutput}
        ${BisonOutput}
        src/ast/declarator.h
        src/ast/declarator.cpp
        src/ast/ast.h
        src/ast/ast.cpp
        src/ast/expression.h
        src/ast/expression.cpp
        src/ast/gen-ctx.h
        src/ast/gen-ctx.cpp
        src/main.cpp)

llvm_map_components_to_libnames(llvm_libs support core irreader)

target_link_libraries(yac ${llvm_libs})