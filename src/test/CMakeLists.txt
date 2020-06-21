set(TARGET test_mpp)

# file
file(GLOB_RECURSE test_mpp ${CMAKE_CURRENT_LIST_DIR}/*.cpp)
add_executable(${TARGET} ${test_mpp})
set_target_properties(${TARGET}  PROPERTIES
                      OUTPUT_NAME "test_mpp")

# include
include_directories(${CMAKE_CURRENT_LIST_DIR})
include_directories(${CMAKE_SOURCE_DIR}/src/rapidcv_mpp)

# link
target_link_libraries(${TARGET} shared_rapidcv_mpp)

# install
set_target_properties(${TARGET} PROPERTIES INSTALL_RPATH "$ORIGIN/../lib")
install(TARGETS ${TARGET} RUNTIME DESTINATION "test")
