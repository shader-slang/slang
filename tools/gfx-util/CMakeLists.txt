add_library(gfx-util shader-cursor.cpp)
set_default_compile_options(gfx-util)
target_link_libraries(gfx-util PRIVATE core)

# For slang-gfx.h
target_include_directories(gfx-util
                           PRIVATE $<TARGET_PROPERTY:gfx,INCLUDE_DIRECTORIES>)
