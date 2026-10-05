cmake_minimum_required(VERSION 3.20)

project("vkcompute" CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(LTO "Enable Link-time Optimization" ON)

file(GLOB_RECURSE SRC CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

set(INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/include")

add_executable(vkcompute ${SRC})

target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE})

find_package(Vulkan REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan)

set(FETCHCONTENT_QUIET FALSE CACHE BOOL "Show FetchContent logs" FORCE)
set(SLP_PNG_STATIC OFF CACHE BOOL "" FORCE)
include(FetchContent)
FetchContent_Declare(
  slp_png
  GIT_REPOSITORY https://github.com/miyabi-no1-fan/slp_png.git
  GIT_TAG v1.2.1
  GIT_SHALLOW TRUE
  GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(slp_png)
target_link_libraries(${PROJECT_NAME} PRIVATE slp_png)

if(LTO)
  include(CheckIPOSupported)
  check_ipo_supported(RESULT ipo_supported OUTPUT ipo_output)
  if(ipo_supported)
    set_target_properties(vkcompute PROPERTIES
      INTERPROCEDURAL_OPTIMIZATION TRUE)
  endif()
endif()
