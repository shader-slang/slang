// Regression test for https://github.com/shader-slang/slang/issues/12718:
// an empty callable-data struct must not be erased on the HLSL/D3D path, or DXC rejects the
// resulting zero-parameter callable. SPIR-V keeps a valid callable entry point without
// materializing the unused empty payload.

//TEST:SIMPLE(filecheck=HLSL): -stage callable -entry callableMain -target hlsl -profile sm_6_7
//TEST:SIMPLE(filecheck=DXIL): -stage callable -entry callableMain -target dxil -profile sm_6_7
//TEST:SIMPLE(filecheck=SPIRV): -stage callable -entry callableMain -target spirv

struct EmptyData
{
}

[shader("callable")]
void callableMain(inout EmptyData data)
{
}

// HLSL-DAG: struct EmptyData
// HLSL-DAG: int _slang_dummy{{.*}};
// HLSL: void callableMain(inout EmptyData{{.*}} {{[A-Za-z0-9_]+}})

// DXIL: define void @

// SPIR-V does not need the D3D dummy padding: an empty callable payload on a callable entry
// point compiles to a valid CallableKHR entry point without materializing the unused payload.
// SPIRV: OpEntryPoint CallableKHR
