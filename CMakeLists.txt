cmake_minimum_required(VERSION 3.5.0)
project(MaBoSS VERSION 0.1.0 LANGUAGES CXX)


if(NOT MAXNODES)
    set(MAXNODES 64)
endif()

if(MAXNODES EQUAL 64)
    set(MABOSS_NAME MaBoSS)
    set(MABOSS_LIBNAME libMaBoSS)
    set(POPMABOSS_NAME PopMaBoSS)
else()
    set(MABOSS_NAME MaBoSS_${MAXNODES}n)
    set(MABOSS_LIBNAME libMaBoSS_${MAXNODES}n)
    set(POPMABOSS_NAME PopMaBoSS_${MAXNODES}n)
endif()

add_executable(${MABOSS_NAME} engine/src/MaBoSS.cc)
add_executable(${POPMABOSS_NAME} engine/src/PopMaBoSS.cc)
add_definitions(-DMAXNODES=${MAXNODES})

find_package(FLEX REQUIRED)
IF(FLEX_FOUND)
    ADD_CUSTOM_COMMAND(
      SOURCE ${CMAKE_SOURCE_DIR}/engine/src/BooleanGrammar.l
      COMMAND ${FLEX_EXECUTABLE}
      ARGS --outfile=${CMAKE_SOURCE_DIR}/engine/src/lex.CTBNDL.cc
        ${CMAKE_SOURCE_DIR}/engine/src/BooleanGrammar.l
      COMMENT "Generating Boolean Network Lexer"
      OUTPUT ${CMAKE_SOURCE_DIR}/engine/src/lex.CTBNDL.cc
    )
    
    ADD_CUSTOM_COMMAND(
      SOURCE ${CMAKE_SOURCE_DIR}/engine/src/RunConfigGrammar.l
      COMMAND ${FLEX_EXECUTABLE}
      ARGS --outfile=${CMAKE_SOURCE_DIR}/engine/src/lex.RC.cc
        ${CMAKE_SOURCE_DIR}/engine/src/RunConfigGrammar.l
      COMMENT "Generating Config Lexer"
      OUTPUT ${CMAKE_SOURCE_DIR}/engine/src/lex.RC.cc
    )
ENDIF(FLEX_FOUND)

find_package(BISON REQUIRED)
set(BISON_ctbndl_parser_OUTPUTS ${CMAKE_SOURCE_DIR}/engine/src/BooleanGrammar.cc)
set(BISON_rc_parser_OUTPUTS ${CMAKE_SOURCE_DIR}/engine/src/RunConfigGrammar.cc)
if(BISON_FOUND)
    ADD_CUSTOM_COMMAND(
      SOURCE ${CMAKE_SOURCE_DIR}/engine/src/BooleanGrammar.y
      COMMAND ${BISON_EXECUTABLE}
      ARGS -v ${CMAKE_SOURCE_DIR}/engine/src/BooleanGrammar.y
           --output=${CMAKE_SOURCE_DIR}/engine/src/BooleanGrammar.cc
      DEPENDS ${CMAKE_SOURCE_DIR}/engine/src/lex.CTBNDL.cc
      COMMENT "Generating BooleanGrammar parser.cpp"
      OUTPUT ${CMAKE_SOURCE_DIR}/engine/src/BooleanGrammar.cc
    )
    
    ADD_CUSTOM_COMMAND(
      SOURCE ${CMAKE_SOURCE_DIR}/engine/src/RunConfigGrammar.y
      COMMAND ${BISON_EXECUTABLE}
      ARGS -v ${CMAKE_SOURCE_DIR}/engine/src/RunConfigGrammar.y
           --output=${CMAKE_SOURCE_DIR}/engine/src/RunConfigGrammar.cc
      DEPENDS ${CMAKE_SOURCE_DIR}/engine/src/lex.RC.cc
      COMMENT "Generating Runconfig parser.cpp"
      OUTPUT ${CMAKE_SOURCE_DIR}/engine/src/RunConfigGrammar.cc
    )
ENDIF(BISON_FOUND)
file(GLOB_RECURSE src_files "engine/src/*.cc")
list(REMOVE_ITEM src_files "PopMaBoSS.cc MaBoSS.cc Server.cc Client.cc BooleanGrammar.cc RunConfig.cc")
add_library(${MABOSS_LIBNAME} STATIC ${src_files} ${BISON_ctbndl_parser_OUTPUTS} ${BISON_rc_parser_OUTPUTS})
target_compile_definitions(${MABOSS_LIBNAME} PUBLIC MAXNODES=${MAXNODES})
target_link_libraries(${MABOSS_NAME} ${MABOSS_LIBNAME})
target_link_libraries(${POPMABOSS_NAME} ${MABOSS_LIBNAME})
