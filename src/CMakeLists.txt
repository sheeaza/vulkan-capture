find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)
include_directories(${GLFW_INCLUDE_DIRS})

find_package (Threads)

find_package(Vulkan REQUIRED)

aux_source_directory(. SOURCE_FILES)
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} ${Vulkan_LIBRARIES}
                                      ${GLFW_STATIC_LIBRARIES}
                                      ${CMAKE_THREAD_LIBS_INIT})

install(TARGETS ${PROJECT_NAME} DESTINATION bin)

find_program(GLSL glslangValidator)
set(shader-src-dir ${CMAKE_CURRENT_SOURCE_DIR})
set(shader-out-dir ${CMAKE_CURRENT_BINARY_DIR})
file(GLOB shaders-path "${shader-src-dir}/*.frag" "${shader-src-dir}/*.vert")
foreach(shader-path ${shaders-path})
    get_filename_component(shader ${shader-path} NAME)
    add_custom_command(
        OUTPUT ${shader-out-dir}/${shader}.spv
        COMMAND ${GLSL} -V ${shader-path} -o ${shader-out-dir}/${shader}.spv
        DEPENDS ${shader-path}
        IMPLICIT_DEPENDS CXX ${shader-path}
        VERBATIM)
set_source_files_properties(${shader-out-dir}/${shader}.spv PROPERTIES GENERATED TRUE)
target_sources(${PROJECT_NAME} PRIVATE ${shader-out-dir}/${shader}.spv)
install(FILES ${shader-out-dir}/${shader}.spv DESTINATION bin)
endforeach(shader-path)

