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
            COMMENT "Generating syntax.cpp"
    )
ENDIF()

add_executable(Basic_Compiler
        #${FlexOutput}
        #${BisonOutput}
        src/ast/ast.h
        src/ast/ast.cpp
        src/ast/expression.h
        src/ast/expression.cpp
        src/ast/gen-ctx.h
        src/ast/gen-ctx.cpp
        src/main.cpp)
