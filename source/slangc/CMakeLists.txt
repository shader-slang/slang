add_executable(slang-bootstrap EXCLUDE_FROM_ALL main.cpp)
set_default_compile_options(slang-bootstrap)
target_link_libraries(slang-bootstrap PRIVATE core slang-static
                                              Threads::Threads)

# The `slangc` command-line application is just a very thin wrapper around the
# Slang dynamic library, so its build is extremely simple. One windows `slangc`
# uses the `core` library for some UTF-16 to UTF-8 string conversion before
# calling into `slang.dll`, so it also depends on `core`:
add_executable(slangc main.cpp)
set_default_compile_options(slangc)
target_link_libraries(slangc PRIVATE core slang Threads::Threads)
