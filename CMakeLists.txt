project(rapidcv_mpp)
cmake_minimum_required(VERSION 2.8)

# cmake flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -fPIC -rdynamic -fpermissive -std=c++11")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -fPIC -rdynamic -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast -Wno-int-conversion -fpermissive")

if(USE_3519)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mfpu=neon-vfpv4 -mfloat-abi=softfp -mcpu=cortex-a53")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mfpu=neon-vfpv4 -mfloat-abi=softfp -mcpu=cortex-a53")
elseif(USE_3516)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mfpu=neon-vfpv4 -mfloat-abi=softfp -mcpu=cortex-a7")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mfpu=neon-vfpv4 -mfloat-abi=softfp -mcpu=cortex-a7")
endif()

# definition
if(USE_3519)
    add_definitions("-DUSE_3519")
elseif(USE_3516)
   add_definitions("-DUSE_3516")
endif()

# thirdparty
include(thirdparty/CMakeLists.txt)

# sub
add_subdirectory(src)


# install
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/release")
endif()
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)