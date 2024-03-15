# https://cliutils.gitlab.io/modern-cmake/chapters/basics.html

cmake_minimum_required(VERSION 3.14)

project(new_ldg VERSION 0.1
  DESCRIPTION "Improved Level-of-Detail Grids"
  LANGUAGES CXX)

# There are several things to unpack here. one is both the name of the executable file generated, and the name of the CMake target created (you'll hear a lot more about targets soon, I promise). The source file list comes next, and you can list as many as you'd like. CMake is smart, and will only compile source file extensions. The headers will be, for most intents and purposes, ignored; the only reason to list them is to get them to show up in IDEs. Targets show up as folders in many IDEs. More about the general build system and targets is available at buildsystem.
add_executable(ldg_core src/supertiles/ldg_core.cpp)
add_executable(new_ldg src/new_ldg.cpp)

# PUBLIC doesn't mean much for an executable; for a library it lets CMake know that any targets that link to this target must also need that include directory.
target_include_directories(ldg_core PUBLIC ..)
target_include_directories(new_ldg PUBLIC ..)

target_compile_features(ldg_core PUBLIC cxx_std_17)
target_compile_features(new_ldg PUBLIC cxx_std_17)

include(FetchContent)

#
# ----- Libraries -----
#

# LDG Helper library
FetchContent_Declare(
  helper
  GIT_REPOSITORY https://github.com/freysn/helper.git
  GIT_TAG master
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
)
FetchContent_MakeAvailable(helper)

add_library(helper INTERFACE)
target_include_directories(helper INTERFACE ${helper_SOURCE_DIR})
target_link_libraries(ldg_core PUBLIC helper)
target_link_libraries(new_ldg PUBLIC helper)

#CXXOpts
FetchContent_Declare(
    cxxopts
    GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
    GIT_TAG v3.2.0
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
)
FetchContent_MakeAvailable(cxxopts)

target_include_directories(cxxopts INTERFACE ${cxxopts_SOURCE_DIR})
target_link_libraries(new_ldg PUBLIC cxxopts)

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
target_link_libraries(ldg_core PUBLIC cimg)
target_link_libraries(new_ldg PUBLIC cimg)

# OpenMP
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(ldg_core PUBLIC OpenMP::OpenMP_CXX)
    target_link_libraries(new_ldg PUBLIC OpenMP::OpenMP_CXX)
else()
    target_compile_definitions(ldg_core PUBLIC NO_OMP=1)
endif()

# PNG
find_package(PNG REQUIRED)
include_directories(${PNG_INCLUDE_DIR})
target_link_libraries(ldg_core PUBLIC ${PNG_LIBRARY})
target_link_libraries(new_ldg PUBLIC ${PNG_LIBRARY})

# BZip2
find_package(BZip2 REQUIRED)
include_directories(${BZIP2_INCLUDE_DIRS})
target_link_libraries(ldg_core PUBLIC ${BZIP2_LIBRARIES})
target_link_libraries(new_ldg PUBLIC ${BZIP2_LIBRARIES})

# X11 - This specifically helps for containers
find_package(X11 REQUIRED)
target_link_libraries(new_ldg PUBLIC X11)

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
target_link_libraries(new_ldg PUBLIC Eigen3::Eigen)

# the `pkg_check_modules` function is created with this call
find_package(PkgConfig REQUIRED)

# these calls create special `PkgConfig::<MODULE>` variables
pkg_check_modules(MY_PKG REQUIRED IMPORTED_TARGET cairo)

target_link_libraries(ldg_core PUBLIC PkgConfig::MY_PKG)
target_link_libraries(new_ldg PUBLIC PkgConfig::MY_PKG)

# Detect if the "port" command is valid on this system; if so, return full path
EXECUTE_PROCESS(COMMAND which port RESULT_VARIABLE DETECT_MACPORTS OUTPUT_VARIABLE MACPORTS_PREFIX ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

IF (${DETECT_MACPORTS} EQUAL 0)
    # "/opt/local/bin/port" doesn't have libs, so we get the parent directory
    GET_FILENAME_COMPONENT(MACPORTS_PREFIX ${MACPORTS_PREFIX} DIRECTORY)

    # "/opt/local/bin" doesn't have libs, so we get the parent directory
    GET_FILENAME_COMPONENT(MACPORTS_PREFIX ${MACPORTS_PREFIX} DIRECTORY)

    # "/opt/local" is where MacPorts lives, add `/lib` suffix and link
    LINK_DIRECTORIES(${LINK_DIRECTORIES} ${MACPORTS_PREFIX}/lib)

    INCLUDE_DIRECTORIES(${INCLUDE_DIRECTORIES} ${MACPORTS_PREFIX}/include)

    MESSAGE("WINNING!: ${MACPORTS_PREFIX}/lib")
ENDIF()
