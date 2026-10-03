cmake_minimum_required(VERSION 3.20)

project("vkcompute" CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(LTO "Enable Link-time Optimization" ON)

file(GLOB_RECURSE SRC CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable(vkcompute ${SRC})

target_include_directories(${PROJECT_NAME} PRIVATE include)

find_package(Vulkan REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan)

if(LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT ipo_supported OUTPUT ipo_output)
    if(ipo_supported)
        set_target_properties(vkcompute PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
endif()
