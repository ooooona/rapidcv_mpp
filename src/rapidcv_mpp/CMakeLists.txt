set(SHARED_LIB shared_rapidcv_mpp)
set(STATIC_LIB static_rapidcv_mpp)

# file
file(GLOB_RECURSE src ${CMAKE_CURRENT_LIST_DIR}/*.c ${CMAKE_CURRENT_LIST_DIR}/*.cpp)
file(GLOB_RECURSE headers ${CMAKE_CURRENT_LIST_DIR}/rapidcv_mpp.h)
## Shared Lib (.so)
add_library(${SHARED_LIB} SHARED ${src})
set_target_properties(${SHARED_LIB} PROPERTIES
                      OUTPUT_NAME "rapidcv_mpp"
                      PUBLIC_HEADER "${headers}")
## Static Lib (.a)
add_library(${STATIC_LIB} STATIC ${src})
set_target_properties(${STATIC_LIB} PROPERTIES
                      OUTPUT_NAME "rapidcv_mpp"
                      PUBLIC_HEADER "${headers}")

## include
target_include_directories(${SHARED_LIB} 
        PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(${STATIC_LIB} 
        PUBLIC ${CMAKE_CURRENT_LIST_DIR})

# libs
target_link_libraries(${SHARED_LIB} ${HISI_LIBS} rt dl)
target_link_libraries(${STATIC_LIB} ${HISI_LIBS} rt dl)

# install
install(TARGETS ${SHARED_LIB} ${STATIC_LIB}
        LIBRARY DESTINATION "lib"
        ARCHIVE DESTINATION "lib"
        PUBLIC_HEADER DESTINATION "include")
