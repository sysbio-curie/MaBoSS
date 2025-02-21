cmake_minimum_required(VERSION 3.5.0)
project(MaBoSS VERSION 0.1.0 LANGUAGES C CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")

if(NOT MAXNODES)
    set(MAXNODES 64)
endif()

if(MAXNODES EQUAL 64)
    set(MABOSS_NAME MaBoSS)
    set(MABOSS_LIBNAME libMaBoSS)
    set(POPMABOSS_NAME PopMaBoSS)
    set(MABOSS_SERVER_NAME MaBoSS-server)
else()
    set(MABOSS_NAME MaBoSS_${MAXNODES}n)
    set(MABOSS_LIBNAME libMaBoSS_${MAXNODES}n)
    set(POPMABOSS_NAME PopMaBoSS_${MAXNODES}n)
    set(MABOSS_SERVER_NAME MaBoSS-server_${MAXNODES}n)
endif()
set(MABOSS_CLIENT_NAME MaBoSS-client)

add_executable(${MABOSS_NAME} engine/src/MaBoSS.cc)
add_executable(${POPMABOSS_NAME} engine/src/PopMaBoSS.cc)
add_executable(${MABOSS_SERVER_NAME} engine/src/Server.cc)
add_executable(${MABOSS_CLIENT_NAME} engine/src/Client.cc)
add_definitions(-DMAXNODES=${MAXNODES})

find_package(FLEX REQUIRED)
IF(FLEX_FOUND)
    ADD_CUSTOM_COMMAND(
      SOURCE ${CMAKE_SOURCE_DIR}/engine/src/BooleanGrammar.l
      COMMAND ${FLEX_EXECUTABLE}
      ARGS --outfile=${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${MABOSS_LIBNAME}.dir/engine/src/lex.CTBNDL.cc
        ${CMAKE_SOURCE_DIR}/engine/src/BooleanGrammar.l
      COMMENT "Generating Boolean Network Lexer"
      OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${MABOSS_LIBNAME}.dir/engine/src/lex.CTBNDL.cc
    )
    
    ADD_CUSTOM_COMMAND(
      SOURCE ${CMAKE_SOURCE_DIR}/engine/src/RunConfigGrammar.l
      COMMAND ${FLEX_EXECUTABLE}
      ARGS --outfile=${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${MABOSS_LIBNAME}.dir/engine/src/lex.RC.cc
        ${CMAKE_SOURCE_DIR}/engine/src/RunConfigGrammar.l
      COMMENT "Generating Config Lexer"
      OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${MABOSS_LIBNAME}.dir/engine/src/lex.RC.cc
    )
ENDIF(FLEX_FOUND)

find_package(BISON REQUIRED)
set(BISON_ctbndl_parser_OUTPUTS ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${MABOSS_LIBNAME}.dir/engine/src/BooleanGrammar.cc)
set(BISON_rc_parser_OUTPUTS ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${MABOSS_LIBNAME}.dir/engine/src/RunConfigGrammar.cc)
if(BISON_FOUND)
    ADD_CUSTOM_COMMAND(
      SOURCE ${CMAKE_SOURCE_DIR}/engine/src/BooleanGrammar.y
      COMMAND ${BISON_EXECUTABLE}
      ARGS -v ${CMAKE_SOURCE_DIR}/engine/src/BooleanGrammar.y
           --output=${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${MABOSS_LIBNAME}.dir/engine/src/BooleanGrammar.cc
      DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${MABOSS_LIBNAME}.dir/engine/src/lex.CTBNDL.cc
      COMMENT "Generating BooleanGrammar parser.cpp"
      OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${MABOSS_LIBNAME}.dir/engine/src/BooleanGrammar.cc
    )
    
    ADD_CUSTOM_COMMAND(
      SOURCE ${CMAKE_SOURCE_DIR}/engine/src/RunConfigGrammar.y
      COMMAND ${BISON_EXECUTABLE}
      ARGS -v ${CMAKE_SOURCE_DIR}/engine/src/RunConfigGrammar.y
           --output=${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${MABOSS_LIBNAME}.dir/engine/src/RunConfigGrammar.cc
      DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${MABOSS_LIBNAME}.dir/engine/src/lex.RC.cc
      COMMENT "Generating Runconfig parser.cpp"
      OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${MABOSS_LIBNAME}.dir/engine/src/RunConfigGrammar.cc
    )
ENDIF(BISON_FOUND)

file(GLOB_RECURSE src_files "engine/src/*.cc")
list(REMOVE_ITEM src_files "PopMaBoSS.cc MaBoSS.cc Server.cc Client.cc BooleanGrammar.cc RunConfig.cc")

add_library(${MABOSS_LIBNAME} STATIC ${src_files} ${BISON_ctbndl_parser_OUTPUTS} ${BISON_rc_parser_OUTPUTS})
target_compile_definitions(${MABOSS_LIBNAME} PUBLIC MAXNODES=${MAXNODES})

if(MPI)
    find_package(MPI)
    if(MPI_CXX_FOUND)
        target_compile_definitions(${MABOSS_LIBNAME} PUBLIC MPI_COMPAT=1)
        target_include_directories(${MABOSS_LIBNAME} PUBLIC ${MPI_CXX_INCLUDE_PATH})
        target_link_libraries(${MABOSS_LIBNAME} ${MPI_CXX_LIBRARIES})
    endif()
endif()

# find_package(HDF5)
# if(HDF5_FOUND)
#     target_compile_definitions(${MABOSS_LIBNAME} PUBLIC HDF5_COMPAT=1)
#     target_include_directories(${MABOSS_LIBNAME} PUBLIC ${HDF5_INCLUDE_DIRS})
#     target_link_libraries(${MABOSS_LIBNAME} ${HDF5_LIBRARIES})
# endif()

find_package(LIBSBML)
if(LIBSBML_FOUND)
    target_compile_definitions(${MABOSS_LIBNAME} PUBLIC SBML_COMPAT=1)
    target_include_directories(${MABOSS_LIBNAME} PUBLIC ${LIBSBML_INCLUDE_DIRS})
    target_link_libraries(${MABOSS_LIBNAME} ${LIBSBML_LIBRARIES})
endif()

target_include_directories(${MABOSS_LIBNAME} PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${MABOSS_LIBNAME}.dir/engine/src)
target_include_directories(${MABOSS_LIBNAME} PUBLIC ${CMAKE_SOURCE_DIR}/engine/src)
target_link_libraries(${MABOSS_NAME} ${MABOSS_LIBNAME})
target_link_libraries(${POPMABOSS_NAME} ${MABOSS_LIBNAME})
target_link_libraries(${MABOSS_SERVER_NAME} ${MABOSS_LIBNAME})
target_link_libraries(${MABOSS_CLIENT_NAME} ${MABOSS_LIBNAME})
