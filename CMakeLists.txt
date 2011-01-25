include(CheckIncludeFile)
include(CheckIncludeFileCxx)

cmake_minimum_required(VERSION 2.6)
project(SIBNET)

# no ppc because it's untested (and do we need it at all?)
set(CMAKE_OSX_ARCHITECTURES "i386;x86_64")

file(GLOB_RECURSE SIBNET_SOURCES ${SIBNET_SOURCE_DIR}/src/*.cpp)
include_directories(${SIBNET_SOURCE_DIR}/include ../HawkNL/include)
link_directories(${SIBNET_SOURCE_DIR}/../HawkNL/src)
add_library(sibnet SHARED ${SIBNET_SOURCES})
target_link_libraries(sibnet -lNL.1.7.0)


# look for stdint and generate wrapper
check_include_file_cxx(cstdint CMAKE_HAVE_CSTDINT)
check_include_file(stdint.h CMAKE_HAVE_STDINT_H)
configure_file(${SIBNET_SOURCE_DIR}/include/stdint_wrap.h.in include/stdint_wrap.h)
include_directories(${SIBNET_BINARY_DIR}/include)

install(TARGETS sibnet DESTINATION lib)
install(DIRECTORY ${SIBNET_SOURCE_DIR}/include/ DESTINATION include/sibnet FILES_MATCHING PATTERN "*.h")
install(DIRECTORY ${SIBNET_BINARY_DIR}/include/ DESTINATION include/sibnet FILES_MATCHING PATTERN "*.h")