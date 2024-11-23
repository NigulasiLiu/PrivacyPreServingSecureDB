CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
project(ppvd)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(LOCAL_LINK_DIR /usr/local/lib)
set(LOCAL_INCLUDE_DIR /usr/local/include)

# ccache
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    set(CMAKE_CUDA_COMPILER_LAUNCHER "${CCACHE_PROGRAM}") # CMake 3.9+
else()
    message(WARNING "can't find ccache.")
endif()
include_directories(
    ${LOCAL_INCLUDE_DIR}/eigen3    
)
link_directories(
    ${LOCAL_LINK_DIR}
    ${PROJECT_SOURCE_DIR}/3rd/pbcwrapper
)

find_package(OpenSSL REQUIRED)
# find_package(Eigen3 3.4 REQUIRED NO_MODULE)
# if(Eigen3_FOUND)
#     message("Eigen found~")
# else()
#     message(FATAL_ERROR "can't find eigen.")
# endif()

# include(FetchContent)
# FetchContent_Declare(
#     googletest
#     GIT_REPOSITORY git@github.com:google/googletest.git
#     GIT_TAG release-1.12.1
# )
# FetchContent_MakeAvailable(googletest)

# add_subdirectory(${PROJECT_SOURCE_DIR}/test)

# option(TEST_QUERY "test in query" OFF)
# if(TEST_QUERY)
#     add_compile_definitions(TEST_QUERY)
#     message("TEST_QUERY")
# endif()

##########################################
#                   exe                  #
##########################################
aux_source_directory(${PROJECT_SOURCE_DIR}/src DIR_SRCS)
add_executable(mainExe
    ${DIR_SRCS}
)

target_include_directories(mainExe
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${OPENSSL_INCLUDE_DIR}
)
find_path(GMP_INCLUDE_DIR NAMES gmp.h)
find_library(GMP_LIBRARIES NAMES gmp libgmp)
find_library(GMPXX_LIBRARIES NAMES gmpxx libgmpxx)
target_link_libraries(mainExe
        PBC
        pbc
        gmp gmpxx
        ntl m
        pthread
        OpenSSL::SSL OpenSSL::Crypto
        # Eigen3::Eigen
        # ${PROJECT_SOURCE_DIR}/3rd/pbcwrapper/libPBC.a
)
