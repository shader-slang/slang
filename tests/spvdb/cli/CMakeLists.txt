add_executable(spvdb-cli main.cpp)

target_link_libraries(spvdb-cli PRIVATE spvdb replxx)

set_target_properties(spvdb-cli PROPERTIES OUTPUT_NAME spvdb)
