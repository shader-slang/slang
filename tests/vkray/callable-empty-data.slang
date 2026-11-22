// Regression test for https://github.com/shader-slang/slang/issues/12718 and its CUDA counterpart,
// #13106. D3D and Khronos receivers use the same one-uint physical payload as their callers.
// The logical empty input still disappears from the body; CUDA/OptiX needs no artificial storage.

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
// HLSL-NEXT: uint _slang_dummy{{.*}};
// HLSL-NEXT: };
// HLSL: void callableMain(inout [[carrier]] {{[A-Za-z0-9_]+}})

// The real signal here is that DXC accepts the DXIL at all: it rejects a zero-parameter callable,
// so a successful compile confirms the parameter survived. This only checks a function is emitted.
// DXIL: define void @

// Preserve the incoming interface even when the empty logical parameter has no body uses.
// SPIRV: OpEntryPoint CallableKHR
// SPIRV: %[[uint:[A-Za-z0-9_]+]] = OpTypeInt 32 0
// SPIRV: %[[physicalType:[A-Za-z0-9_]+]] = OpTypeStruct %[[uint]]{{$}}
// SPIRV: %[[pointer:[A-Za-z0-9_]+]] = OpTypePointer IncomingCallableDataKHR %[[physicalType]]
// SPIRV: = OpVariable %[[pointer]] IncomingCallableDataKHR

// GLSL uses the same one-uint incoming object and keeps an ordinary parameterless entry point.
// GLSL: struct [[physicalType:[A-Za-z0-9_]+]]
// GLSL-NEXT: {
// GLSL-NEXT: uint _slang_dummy{{[A-Za-z0-9_]*}};
// GLSL-NEXT: };
// GLSL: callableDataInEXT
// GLSL-NEXT: [[physicalType]] {{[A-Za-z0-9_]+}};
// GLSL: void main(

// The OptiX callable ABI is variadic, so the empty logical parameter is erased and no artificial
// dummy storage is emitted. A non-empty control is covered by callable-nonempty-data.slang.
// CUDA-NOT: _slang_dummy
// CUDA: void __direct_callable__callableMain()
