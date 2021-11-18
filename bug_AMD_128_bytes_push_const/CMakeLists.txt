cmake_minimum_required(VERSION 2.8.12)

project("example_minimal")

OPTION(USE_WAYLAND_LINUX "use Wayland for Linux" OFF)
OPTION(GEN_WAYLAND_HEADERS "using wayland-scanner" OFF)

find_package(Vulkan REQUIRED)
if(NOT Vulkan_FOUND)
    MESSAGE("ERROR: Vulkan not found")
endif(NOT Vulkan_FOUND)
set(Vulkan_LIBRARY)

set(Wayland_client "")
IF (NOT WIN32)
if(USE_WAYLAND_LINUX)
  set(WAYLAND_INCLUDE_DIR "/usr/include/wayland")
  MESSAGE(STATUS "  Wayland include path ${WAYLAND_INCLUDE_DIR}")
  include_directories(${WAYLAND_INCLUDE_DIR})
  if(GEN_WAYLAND_HEADERS)
    find_program(PKG_CONFIG pkg-config)
    if (NOT PKG_CONFIG)
      MESSAGE(FATAL_ERROR "pkg-config binary not found")
    endif ()
    find_program(WAYLAND_SCANNER wayland-scanner)
    if (NOT WAYLAND_SCANNER)
      MESSAGE(FATAL_ERROR "wayland-scanner binary not found")
    endif ()
    if(PKG_CONFIG AND WAYLAND_SCANNER)
      set(Wayland_client "${CMAKE_BINARY_DIR}/xdg-shell-client-protocol.c")
      execute_process(COMMAND ${PKG_CONFIG} --variable=pkgdatadir wayland-protocols OUTPUT_VARIABLE protocol_dir OUTPUT_STRIP_TRAILING_WHITESPACE)
      execute_process(COMMAND ${WAYLAND_SCANNER} client-header ${protocol_dir}/stable/xdg-shell/xdg-shell.xml xdg-shell-client-protocol.h
          COMMAND ${WAYLAND_SCANNER} private-code ${protocol_dir}/stable/xdg-shell/xdg-shell.xml xdg-shell-client-protocol.c)
      include_directories(${CMAKE_BINARY_DIR})
    endif ()
  else ()
    MESSAGE(STATUS "using wayland headers from wayland_xdg folder")
    set(Wayland_client "../wayland_xdg/xdg-shell-client-protocol.c")
    include_directories("../wayland_xdg")
  endif ()
endif ()
endif ()

if (Vulkan_FOUND)
    MESSAGE(STATUS "  [Found Vulkan library]")
    MESSAGE(STATUS "  Vulkan include directory:" ${Vulkan_INCLUDE_DIR})
    FOREACH(item ${Vulkan_LIBRARIES})
        MESSAGE(STATUS "  using lib: " ${item})
    ENDFOREACH()
    include_directories(${Vulkan_INCLUDE_DIR})
endif()

set(SOURCES
  "main.c" 
  "../vk_utils/vk_utils.c"
  "../vk_utils/vk_error_print.c"
  "../vk_utils/vk_render_helper.c"
  ${Wayland_client}
)

add_executable(
    Vkexample
    ${SOURCES} 
)


add_custom_command(TARGET Vkexample POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_directory
                    ${CMAKE_SOURCE_DIR}/shaders/spv $<TARGET_FILE_DIR:Vkexample>/shaders/spv)

if("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")

  add_definitions(-DVK_USE_PLATFORM_WIN32_KHR )
  target_link_libraries(Vkexample ${Vulkan_LIBRARY})
elseif("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
  if(USE_WAYLAND_LINUX)
    add_definitions(-DVK_USE_PLATFORM_WAYLAND_KHR -O2 -s -fdata-sections -ffunction-sections -Wl,--gc-sections)
    target_link_libraries(Vkexample m wayland-client ${Vulkan_LIBRARY})
  else()
    add_definitions(-DVK_USE_PLATFORM_XCB_KHR -O2 -s -fdata-sections -ffunction-sections -Wl,--gc-sections)
    target_link_libraries(Vkexample m xcb ${Vulkan_LIBRARY})
  endif()
else()
  message(WARNING "Unknown compiler '${CMAKE_C_COMPILER_ID}'!")
endif()
