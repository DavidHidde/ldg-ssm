cmake_minimum_required(VERSION 3.14)

project(
    ldg_ssm
    VERSION 1.0
    DESCRIPTION "Level-of-Detail Self-Sorting Map Grids"
    LANGUAGES CXX
)

add_executable(ldg_ssm src/ldg_ssm.cpp)

target_include_directories(ldg_ssm PRIVATE ..)
target_compile_features(ldg_ssm PRIVATE cxx_std_20)

include(FetchContent)

#
# ----- Libraries -----
#

# CXXOpts
FetchContent_Declare(
    cxxopts
    GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
    GIT_TAG v3.2.0
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
)
FetchContent_MakeAvailable(cxxopts)

target_include_directories(cxxopts INTERFACE ${cxxopts_SOURCE_DIR})
target_link_libraries(ldg_ssm PRIVATE cxxopts)

# JSON
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)
target_link_libraries(ldg_ssm PRIVATE nlohmann_json::nlohmann_json)

# CImg
FetchContent_Declare(
    cimg
    GIT_REPOSITORY https://github.com/GreycLab/CImg.git
    GIT_TAG v.3.3.5
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
)
FetchContent_MakeAvailable(cimg)

add_library(cimg INTERFACE)
target_include_directories(cimg INTERFACE ${cimg_SOURCE_DIR})
target_link_libraries(ldg_ssm PRIVATE cimg)

# OpenMP
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(ldg_ssm PRIVATE OpenMP::OpenMP_CXX)
endif()

# PNG
find_package(PNG REQUIRED)
include_directories(${PNG_INCLUDE_DIR})
target_link_libraries(ldg_ssm PRIVATE ${PNG_LIBRARY})

# BZip2
find_package(BZip2 REQUIRED)
include_directories(${BZIP2_INCLUDE_DIRS})
target_link_libraries(ldg_ssm PRIVATE ${BZIP2_LIBRARIES})

# X11 - This specifically helps for containers
find_package(X11 REQUIRED)
target_link_libraries(ldg_ssm PRIVATE X11)

# Eigen
FetchContent_Declare(
    eigen
    GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
    GIT_TAG 3.4.0
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
)
set(BUILD_TESTING OFF)
set(EIGEN_BUILD_TESTING OFF)
set(EIGEN_MPL2_ONLY ON)
set(EIGEN_BUILD_PKGCONFIG OFF)
set(EIGEN_BUILD_DOC OFF)
FetchContent_MakeAvailable(eigen)
target_link_libraries(ldg_ssm PRIVATE Eigen3::Eigen)
