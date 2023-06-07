cmake_minimum_required (VERSION 3.10)
project(PBD)

set (PBD_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set (PBD_INCLUDE_DIR ${PBD_BASE_DIR}/inc)
set (PBD_SRC_DIR ${PBD_BASE_DIR}/src)
set (THIRD_BASE_DIR ${PBD_BASE_DIR}/third)
set (THIRD_INCLUDE_DIR ${THIRD_BASE_DIR}/inc)
set (THIRD_SRC_DIR ${THIRD_BASE_DIR}/src)
set (THIRD_LIB_DIR ${THIRD_BASE_DIR}/lib)

include_directories(
	${PBD_INCLUDE_DIR}
	${THIRD_INCLUDE_DIR})

aux_source_directory(${PBD_SRC_DIR} PBD_SRCS)
aux_source_directory(${THIRD_SRC_DIR} THIRD_SRCS)

add_executable(PBD ${PBD_SRCS} ${THIRD_SRCS})

set (THIRD_LIBS ${THIRD_LIB_DIR}/glfw3.lib;opengl32.lib)
target_link_libraries(PBD ${THIRD_LIBS})