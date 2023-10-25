add_library(
  platform gui.cpp linux/x11-key-code.cpp linux/x11-window.cpp model.cpp
           placeholder/placeholder-window.cpp windows/win-window.cpp)
set_default_compile_options(platform)
target_link_libraries(platform PRIVATE core)

# For slang-gfx.h
target_include_directories(platform
                           PRIVATE $<TARGET_PROPERTY:gfx,INCLUDE_DIRECTORIES>)

get_target_property(SLANG_PLATFORM_TARGET_TYPE platform TYPE)
if(SLANG_PLATFORM_TARGET_TYPE STREQUAL SHARED_LIBRARY)
  target_compile_definitions(platform PRIVATE SLANG_PLATFORM_DYNAMIC
                                              SLANG_PLATFORM_DYNAMIC_EXPORT)
endif()

if(SLANG_ENABLE_XLIB)
  find_package(X11 REQUIRED)
  target_link_libraries(platform PRIVATE X11::X11)
  target_compile_definitions(platform PRIVATE SLANG_ENABLE_XLIB)
endif()
