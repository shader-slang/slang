// Test that array-typed varying parameters legalize correctly.
// Exercises the array-recursion path in `_createLegalVaryingVal`
// (`slang-ir-legalize-varying-params.cpp`) which handles the "outer
// array brackets" tracking when decomposing a varying parameter type
// into per-element registers.
//
// The PR's other vertex/fragment tests use scalar / vector / struct
// varyings only; array-typed varyings hit a separate recursion path.

//TEST:SIMPLE(filecheck=CHECK_HLSL): -target hlsl -entry vertexMain -stage vertex
//TEST:SIMPLE(filecheck=CHECK_SPV): -target spirv-asm -entry vertexMain -stage vertex -emit-spirv-directly

struct VertexOutput
{
    // Array-typed varying output — legalization must enumerate each
    // element with its own location/semantic.
    float values[4] : OUT_VALUES;
    float4 position : SV_Position;
};

[shader("vertex")]
VertexOutput vertexMain(
    float4 pos : POSITION,
    // Array-typed varying input — same recursion path on the input
    // side.
    float inValues[4] : IN_VALUES)
{
    VertexOutput o;
    [ForceUnroll]
    for (int i = 0; i < 4; ++i)
    {
        o.values[i] = inValues[i] + float(i);
    }
    o.position = pos;
    return o;
}

// HLSL emission preserves the user-declared semantics on the
// struct field and on the function parameter. Both must round-trip
// through the array-recursion legalization path.
// CHECK_HLSL-DAG: values_0[int(4)] : OUT_VALUES
// CHECK_HLSL-DAG: position_0 : SV_Position
// CHECK_HLSL-DAG: inValues_0[int(4)] : IN_VALUES
// CHECK_HLSL-DAG: o_0.values_0[int(0)] = inValues_0[int(0)]
// CHECK_HLSL-DAG: o_0.values_0[int(1)] = inValues_0[int(1)] + 1.0f
// CHECK_HLSL-DAG: o_0.values_0[int(2)] = inValues_0[int(2)] + 2.0f
// CHECK_HLSL-DAG: o_0.values_0[int(3)] = inValues_0[int(3)] + 3.0f

// SPIRV emits one Location decoration per array element (or per
// the top-level array, depending on the SPIR-V Location-bumping
// rules). Either way, the entry point + Location decorations are
// emitted.
// CHECK_SPV-DAG: OpEntryPoint Vertex %vertexMain "main" %entryPointParam_vertexMain_values %gl_Position %pos %inValues
// CHECK_SPV-DAG: OpDecorate %pos Location 0
// CHECK_SPV-DAG: OpDecorate %inValues Location 1
// CHECK_SPV-DAG: OpDecorate %entryPointParam_vertexMain_values Location 0
// CHECK_SPV-DAG: OpDecorate %gl_Position BuiltIn Position
// CHECK_SPV-DAG: %_arr_float_int_4 = OpTypeArray %float %int_4
// CHECK_SPV-DAG: %_ptr_Input__arr_float_int_4 = OpTypePointer Input %_arr_float_int_4
// CHECK_SPV-DAG: %_ptr_Output__arr_float_int_4 = OpTypePointer Output %_arr_float_int_4
// CHECK_SPV-DAG: %inValues = OpVariable %_ptr_Input__arr_float_int_4 Input
// CHECK_SPV-DAG: %entryPointParam_vertexMain_values = OpVariable %_ptr_Output__arr_float_int_4 Output
// CHECK_SPV: OpCompositeConstruct %_arr_float_int_4
// CHECK_SPV: OpStore %entryPointParam_vertexMain_values
// CHECK_SPV: OpStore %gl_Position
