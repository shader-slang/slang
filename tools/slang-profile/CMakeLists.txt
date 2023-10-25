add_executable(slang-profile EXCLUDE_FROM_ALL slang-profile-main.cpp)
set_default_compile_options(slang-profile)
target_link_libraries(slang-profile PRIVATE slang-static)
