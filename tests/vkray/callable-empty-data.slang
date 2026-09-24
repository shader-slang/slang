// Regression test for https://github.com/shader-slang/slang/issues/12718 and its CUDA counterpart,
// #13106. D3D requires a physical carrier for an empty callable-data entry-point parameter;
// CUDA/OptiX, SPIR-V, and GLSL all permit the logically empty payload to contribute no
// physical entry-point parameter.

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

// HLSL: struct [[carrier:[A-Za-z0-9_]+]]
// HLSL-NEXT: {
// HLSL-NEXT: int _slang_dummy{{.*}};
// HLSL-NEXT: };
// HLSL: void callableMain(inout [[carrier]] {{[A-Za-z0-9_]+}})

// The real signal here is that DXC accepts the DXIL at all: it rejects a zero-parameter callable,
// so a successful compile confirms the parameter survived. This only checks a function is emitted.
// DXIL: define void @

// SPIR-V does not need the D3D carrier here: an empty callable payload on a callable entry point
// compiles to a valid CallableKHR entry point without materializing the payload at all. The
// negative checks pin that — no incoming callable-data variable and no dummy field are emitted —
// because no surviving operation needs an incoming interface object here.
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

// The OptiX callable ABI is variadic, so the empty logical parameter is erased and no artificial
// dummy storage is emitted. A non-empty control is covered by callable-nonempty-data.slang.
// CUDA-NOT: _slang_dummy
// CUDA: void __direct_callable__callableMain()
