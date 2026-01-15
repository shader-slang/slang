//TEST:SIMPLE(filecheck=FUNC1): -target spirv -O0 -entry func1
//TEST:SIMPLE(filecheck=FUNC2): -target spirv -O0 -entry func2

// FUNC1: OpEntryPoint Vertex %func1 "main" %entryPointParam_func1_value %entryPointParam_func1_hasValue
// FUNC1-DAG: OpDecorate %entryPointParam_func1_value Location 0
// FUNC1-DAG: OpDecorate %entryPointParam_func1_hasValue Location 1
// FUNC1-DAG: %entryPointParam_func1_value = OpVariable %_ptr_Output_int Output
// FUNC1-DAG: %entryPointParam_func1_hasValue = OpVariable %_ptr_Output_int Output
// FUNC1: OpStore %entryPointParam_func1_value
// FUNC1: OpStore %entryPointParam_func1_hasValue
[shader("vertex")]
Optional<int> func1()
{
    return {};
}

// FUNC2: OpEntryPoint Vertex %func2 "main" %entryPointParam_func2_value_value %entryPointParam_func2_value_hasValue %entryPointParam_func2_hasValue
// FUNC2-DAG: OpDecorate %entryPointParam_func2_value_value Location 0
// FUNC2-DAG: OpDecorate %entryPointParam_func2_value_hasValue Location 1
// FUNC2-DAG: OpDecorate %entryPointParam_func2_hasValue Location 2
// FUNC2-DAG: %entryPointParam_func2_value_value = OpVariable %_ptr_Output_int Output
// FUNC2-DAG: %entryPointParam_func2_value_hasValue = OpVariable %_ptr_Output_int Output
// FUNC2-DAG: %entryPointParam_func2_hasValue = OpVariable %_ptr_Output_int Output
// FUNC2: OpStore %entryPointParam_func2_value_value
// FUNC2: OpStore %entryPointParam_func2_value_hasValue
// FUNC2: OpStore %entryPointParam_func2_hasValue
[shader("vertex")]
Optional<Optional<int>> func2()
{
    return {};
}
