// Floating-point atomic min/max on GLSL are defined by GL_EXT_shader_atomic_float2, which also
// holds every scalar half atomic. GL_EXT_shader_atomic_float only covers 32/64-bit float
// add/exchange/load/store. glslang rejects the shader when the wrong extension is requested.
// https://github.com/shader-slang/slang/issues/14026

//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry floatMain -stage compute
//TEST:SIMPLE(filecheck=SPV): -target spirv-asm -emit-spirv-via-glsl -entry floatMain -stage compute
//TEST:SIMPLE(filecheck=WIDE): -target spirv-asm -emit-spirv-via-glsl -entry halfDoubleMain -stage compute
//TEST:SIMPLE(filecheck=ADD): -target glsl -entry addMain -stage compute
//TEST:SIMPLE(filecheck=HALF): -target glsl -entry halfAddMain -stage compute
//TEST:SIMPLE(filecheck=HALFSPV): -target spirv-asm -emit-spirv-via-glsl -entry halfAddMain -stage compute

RWStructuredBuffer<float> floatBuffer;
RWStructuredBuffer<Atomic<float>> atomicFloatBuffer;
RWTexture2D<float> floatImage;
RWStructuredBuffer<half> halfBuffer;
RWStructuredBuffer<double> doubleBuffer;
RWStructuredBuffer<Atomic<half>> atomicHalfBuffer;
RWStructuredBuffer<Atomic<double>> atomicDoubleBuffer;

// GLSL-NOT: GL_EXT_shader_atomic_float :
// GLSL: #extension GL_EXT_shader_atomic_float2 : require
// GLSL-NOT: GL_EXT_shader_atomic_float :
// GLSL: atomicMin(
// GLSL: atomicMax(
// GLSL: imageAtomicMin(
// GLSL: imageAtomicMax(

// SPV-DAG: OpCapability AtomicFloat32MinMaxEXT
// SPV-DAG: OpAtomicFMinEXT
// SPV-DAG: OpAtomicFMaxEXT

[numthreads(16, 1, 1)]
void floatMain(uint tid: SV_DispatchThreadID)
{
    float v = float(int(tid) - 8) * 0.5;
    InterlockedMin(floatBuffer[0], v);
    InterlockedMax(floatBuffer[1], v);
    atomicFloatBuffer[0].min(v);
    atomicFloatBuffer[1].max(v);
    atomicFloatBuffer[2].reduceMin(v);
    atomicFloatBuffer[3].reduceMax(v);
    InterlockedMin(floatImage[uint2(0, 0)], v);
    InterlockedMax(floatImage[uint2(1, 0)], v);
}

// WIDE-DAG: OpCapability AtomicFloat16MinMaxEXT
// WIDE-DAG: OpCapability AtomicFloat64MinMaxEXT
// WIDE-DAG: OpAtomicFMinEXT
// WIDE-DAG: OpAtomicFMaxEXT

[numthreads(1, 1, 1)]
void halfDoubleMain(uint tid: SV_DispatchThreadID)
{
    InterlockedMin(halfBuffer[0], half(tid));
    InterlockedMax(halfBuffer[1], half(tid));
    InterlockedMin(doubleBuffer[0], double(tid));
    InterlockedMax(doubleBuffer[1], double(tid));
    atomicHalfBuffer[0].min(half(tid));
    atomicHalfBuffer[1].max(half(tid));
    atomicDoubleBuffer[0].min(double(tid));
    atomicDoubleBuffer[1].max(double(tid));
}

// ADD-NOT: GL_EXT_shader_atomic_float2
// ADD: #extension GL_EXT_shader_atomic_float : require
// ADD-NOT: GL_EXT_shader_atomic_float2
// ADD: atomicAdd(

[numthreads(1, 1, 1)]
void addMain(uint tid: SV_DispatchThreadID)
{
    InterlockedAdd(floatBuffer[0], float(tid));
}

// HALF-NOT: GL_EXT_shader_atomic_float :
// HALF: #extension GL_EXT_shader_atomic_float2 : require
// HALF-NOT: GL_EXT_shader_atomic_float :
// HALF: atomicAdd(
// HALF: atomicExchange(

// HALFSPV-DAG: OpCapability AtomicFloat16AddEXT
// HALFSPV-DAG: OpAtomicFAddEXT

[numthreads(1, 1, 1)]
void halfAddMain(uint tid: SV_DispatchThreadID)
{
    InterlockedAdd(halfBuffer[0], half(tid));
    half orig;
    InterlockedExchange(halfBuffer[1], half(tid), orig);
    halfBuffer[2] = orig;
}
