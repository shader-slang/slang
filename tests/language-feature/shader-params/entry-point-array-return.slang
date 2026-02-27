// Returning an Array<Struct, N> from an entry point requires SOA decomposition
// during GLSL/SPIR-V legalization: each struct field becomes a separate
// array-typed output variable.

//TEST:SIMPLE(filecheck=SPV_SINGLE): -target spirv -O0 -skip-spirv-validation -entry singleField
//TEST:SIMPLE(filecheck=HLSL_SINGLE): -target hlsl -O0 -entry singleField -stage vertex
//TEST:SIMPLE(filecheck=GLSL_SINGLE): -target glsl -O0 -entry singleField -stage vertex
//TEST:SIMPLE(filecheck=MTL_SINGLE): -target metal -O0 -entry singleField -stage vertex
//TEST:SIMPLE(filecheck=WGSL_SINGLE): -target wgsl -O0 -entry singleField -stage vertex

//TEST:SIMPLE(filecheck=SPV_MULTI): -target spirv -O0 -skip-spirv-validation -entry multiField
//TEST:SIMPLE(filecheck=HLSL_MULTI): -target hlsl -O0 -entry multiField -stage vertex
//TEST:SIMPLE(filecheck=GLSL_MULTI): -target glsl -O0 -entry multiField -stage vertex
//TEST:SIMPLE(filecheck=MTL_MULTI): -target metal -O0 -entry multiField -stage vertex
//TEST:SIMPLE(filecheck=WGSL_MULTI): -target wgsl -O0 -entry multiField -stage vertex

//TEST:SIMPLE(filecheck=SPV_NESTED): -target spirv -O0 -skip-spirv-validation -entry nestedArray
//TEST:SIMPLE(filecheck=HLSL_NESTED): -target hlsl -O0 -entry nestedArray -stage vertex
//TEST:SIMPLE(filecheck=GLSL_NESTED): -target glsl -O0 -entry nestedArray -stage vertex
//TEST:SIMPLE(filecheck=MTL_NESTED): -target metal -O0 -entry nestedArray -stage vertex
//TEST:SIMPLE(filecheck=WGSL_NESTED): -target wgsl -O0 -entry nestedArray -stage vertex

struct SingleFieldStruct
{
    int a;
}

// SPV_SINGLE: OpEntryPoint Vertex %singleField "main" %entryPointParam_singleField_a
// SPV_SINGLE-DAG: %entryPointParam_singleField_a = OpVariable %_ptr_Output__arr_int_int_1 Output
// SPV_SINGLE: OpStore %entryPointParam_singleField_a

// HLSL_SINGLE: void singleField
// HLSL_SINGLE: out SingleFieldStruct{{.*}} _S{{[0-9]+}}[int(1)]

// GLSL_SINGLE: out int {{.*}}entryPointParam_singleField_a{{.*}}[1]

// MTL_SINGLE: singleField
// MTL_SINGLE: array<SingleFieldStruct{{.*}}, int(1)>

// WGSL_SINGLE: fn singleField
// WGSL_SINGLE: array<SingleFieldStruct{{.*}}, i32(1)>

[shader("vertex")]
Array<SingleFieldStruct, 1> singleField()
{
    return Array<SingleFieldStruct, 1>();
}

struct MultiFieldStruct
{
    int a;
    float b;
}

// SPV_MULTI: OpEntryPoint Vertex %multiField "main" %entryPointParam_multiField_a %entryPointParam_multiField_b
// SPV_MULTI-DAG: %entryPointParam_multiField_a = OpVariable %_ptr_Output__arr_int_int_3 Output
// SPV_MULTI-DAG: %entryPointParam_multiField_b = OpVariable %_ptr_Output__arr_float_int_3 Output
// SPV_MULTI: OpStore %entryPointParam_multiField_a
// SPV_MULTI: OpStore %entryPointParam_multiField_b

// HLSL_MULTI: void multiField
// HLSL_MULTI: out MultiFieldStruct{{.*}} _S{{[0-9]+}}[int(3)]

// GLSL_MULTI-DAG: out int {{.*}}entryPointParam_multiField_a{{.*}}[3]
// GLSL_MULTI-DAG: out float {{.*}}entryPointParam_multiField_b{{.*}}[3]

// MTL_MULTI: multiField
// MTL_MULTI: array<MultiFieldStruct{{.*}}, int(3)>

// WGSL_MULTI: fn multiField
// WGSL_MULTI: array<MultiFieldStruct{{.*}}, i32(3)>

[shader("vertex")]
Array<MultiFieldStruct, 3> multiField()
{
    Array<MultiFieldStruct, 3> result;
    result[0].a = 1; result[0].b = 1.0;
    result[1].a = 2; result[1].b = 2.0;
    result[2].a = 3; result[2].b = 3.0;
    return result;
}

struct NestedStruct
{
    int a;
    float b;
}

// SPV_NESTED: OpEntryPoint Vertex %nestedArray "main" %entryPointParam_nestedArray_a %entryPointParam_nestedArray_b
// SPV_NESTED-DAG: %entryPointParam_nestedArray_a = OpVariable %_ptr_Output__arr__arr_int_int_2_int_3 Output
// SPV_NESTED-DAG: %entryPointParam_nestedArray_b = OpVariable %_ptr_Output__arr__arr_float_int_2_int_3 Output
// SPV_NESTED: OpStore %entryPointParam_nestedArray_a
// SPV_NESTED: OpStore %entryPointParam_nestedArray_b

// HLSL_NESTED: void nestedArray
// HLSL_NESTED: out NestedStruct{{.*}} _S{{[0-9]+}}[int(3)][int(2)]

// GLSL_NESTED-DAG: out int {{.*}}entryPointParam_nestedArray_a{{.*}}[3][2]
// GLSL_NESTED-DAG: out float {{.*}}entryPointParam_nestedArray_b{{.*}}[3][2]

// MTL_NESTED: nestedArray
// MTL_NESTED: array<array<NestedStruct{{.*}}, int(2)>, int(3)>

// WGSL_NESTED: fn nestedArray
// WGSL_NESTED: array<array<NestedStruct{{.*}}, i32(2)>, i32(3)>

[shader("vertex")]
Array<Array<NestedStruct, 2>, 3> nestedArray()
{
    Array<Array<NestedStruct, 2>, 3> result;
    return result;
}
