// Regression test for https://github.com/shader-slang/slang/issues/12718 (and the CUDA counterpart
// #13106): an empty callable-data struct must not be erased on the HLSL/D3D and CUDA paths, where
// the callable data is a fixed-shape entry-point parameter. On D3D, erasure leaves a zero-parameter
// callable that DXC rejects; on CUDA the parameter must survive so it matches the padded payload a
// `CallShader` caller passes (each side pads independently, so they agree by construction). The
// Vulkan targets (SPIR-V and GLSL) keep a valid callable entry point without materializing the
// unused empty payload — no padding needed there.

//TEST:SIMPLE(filecheck=HLSL): -stage callable -entry callableMain -target hlsl -profile sm_6_7
//TEST:SIMPLE(filecheck=DXIL): -stage callable -entry callableMain -target dxil -profile sm_6_7
//TEST:SIMPLE(filecheck=SPIRV): -stage callable -entry callableMain -target spirv
//TEST:SIMPLE(filecheck=GLSL): -stage callable -entry callableMain -target glsl
//TEST:SIMPLE(filecheck=CUDA): -stage callable -entry callableMain -target cuda

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

// The real signal here is that DXC accepts the DXIL at all: it rejects a zero-parameter callable,
// so a successful compile confirms the parameter survived. This only checks a function is emitted.
// DXIL: define void @

// SPIR-V does not need the D3D dummy padding: an empty callable payload on a callable entry point
// compiles to a valid CallableKHR entry point without materializing the payload at all. The
// negative checks pin that — no incoming callable-data variable and no dummy field are emitted —
// so the Vulkan padding pass (which does match the incoming decoration) leaves this entry point
// untouched, because no such global exists for it to find.
// SPIRV: OpEntryPoint CallableKHR
// SPIRV-NOT: IncomingCallableData
// SPIRV-NOT: _slang_dummy

// GLSL likewise emits no payload global for the empty entry-point parameter: the #version header
// confirms a successful compile, no callable-data global or dummy field appears before the entry
// function, and `void main()` confirms the entry point itself survived.
// GLSL: #version
// GLSL-NOT: callableDataInEXT
// GLSL-NOT: _slang_dummy
// GLSL: void main(

// CUDA needs the padding like D3D: the callable entry point keeps its parameter as a padded
// `EmptyData` pointer, so a `CallShader` caller (which pads its payload identically) agrees on the
// callable-data ABI.
// CUDA-DAG: int _slang_dummy
// CUDA: void __direct_callable__callableMain(EmptyData{{.*}}*{{.*}})
