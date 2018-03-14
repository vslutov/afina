# add dependencies
include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/include)

add_subdirectory(allocator)
add_subdirectory(coroutine)
add_subdirectory(execute)
add_subdirectory(protocol)
add_subdirectory(network)
add_subdirectory(storage)

# Generate version file
set(version_file "${CMAKE_CURRENT_BINARY_DIR}/Version.cpp")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Version.cpp.in ${version_file})

# Executor
add_library(Executor Executor.cpp)
target_link_libraries(Executor ${CMAKE_THREAD_LIBS_INIT})

# build service
set(SOURCE_FILES main.cpp ${version_file})
add_executable(afina ${SOURCE_FILES} ${BACKWARD_ENABLE})
target_link_libraries(afina Network Storage cxxopts Executor)
add_backward(afina)
