//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=RESULT): -cpu -compute -output-using-type -xslang -O0
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=RESULT): -cuda -compute -output-using-type -xslang -O0
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=RESULT): -llvm -compute -output-using-type -xslang -O0
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=RESULT): -vk -compute -output-using-type -xslang -O0
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=RESULT): -wgpu -compute -output-using-type -xslang -O0
//TEST(compute, metal):COMPARE_COMPUTE(filecheck-buffer=RESULT): -metal -compute -output-using-type -xslang -O0
//TEST:SIMPLE(filecheck=NATIVE): -target hlsl -entry computeMain -stage compute -validate-ir -O0
//TEST:SIMPLE(filecheck=NATIVE): -target metal -entry computeMain -stage compute -validate-ir -O0
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry computeMain -stage compute -validate-ir -O0
//TEST:SIMPLE(filecheck=SPIRV): -target wgsl-spirv-asm -entry computeMain -stage compute -validate-ir -O0
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -entry computeMain -stage compute -validate-ir -O0

// Sized and unsized buffers remain distinct source types even when a target gives them the same
// resource representation. The generic comparison must finish before that representation is chosen.
[noinline]
uint sameType<T, U>()
{
    return uint(T is U);
}

struct Wrapper<T>
{
    T value;
}

//TEST_INPUT:ubuffer(data=[9 9 9 9 9 9 9 9 9 9 9 9 9 9 9 9 9 9 9 9], stride=4):out,name=output
RWStructuredBuffer<uint> output;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Neither direction of a sized/unsized comparison is equal.
    output[0] = sameType<__UnsizedStructuredBuffer<uint>, StructuredBuffer<uint>>();
    output[1] = sameType<__UnsizedRWStructuredBuffer<uint>, RWStructuredBuffer<uint>>();
    output[2] = sameType<__UnsizedByteAddressBuffer, ByteAddressBuffer>();
    output[3] = sameType<__UnsizedRWByteAddressBuffer, RWByteAddressBuffer>();
    output[4] = sameType<StructuredBuffer<uint>, __UnsizedStructuredBuffer<uint>>();
    output[5] = sameType<RWStructuredBuffer<uint>, __UnsizedRWStructuredBuffer<uint>>();
    output[6] = sameType<ByteAddressBuffer, __UnsizedByteAddressBuffer>();
    output[7] = sameType<RWByteAddressBuffer, __UnsizedRWByteAddressBuffer>();

    // Identical types and the provisional sized aliases must still compare equal.
    output[8] = sameType<__UnsizedStructuredBuffer<uint>, __UnsizedStructuredBuffer<uint>>();
    output[9] = sameType<__UnsizedRWStructuredBuffer<uint>, __UnsizedRWStructuredBuffer<uint>>();
    output[10] = sameType<__UnsizedByteAddressBuffer, __UnsizedByteAddressBuffer>();
    output[11] = sameType<__UnsizedRWByteAddressBuffer, __UnsizedRWByteAddressBuffer>();
    output[12] = sameType<StructuredBuffer<uint>, __SizedStructuredBuffer<uint>>();
    output[13] = sameType<RWStructuredBuffer<uint>, __SizedRWStructuredBuffer<uint>>();
    output[14] = sameType<ByteAddressBuffer, __SizedByteAddressBuffer>();
    output[15] = sameType<RWByteAddressBuffer, __SizedRWByteAddressBuffer>();

    // The distinction also survives specialization of a surrounding generic type.
    typealias SizedWrapper = Wrapper<Wrapper<StructuredBuffer<uint>>>;
    typealias UnsizedWrapper = Wrapper<Wrapper<__UnsizedStructuredBuffer<uint>>>;
    typealias SizedAliasWrapper = Wrapper<Wrapper<__SizedStructuredBuffer<uint>>>;
    output[16] = sameType<SizedWrapper, UnsizedWrapper>();
    output[17] = sameType<UnsizedWrapper, SizedWrapper>();
    output[18] = sameType<UnsizedWrapper, UnsizedWrapper>();
    output[19] = sameType<SizedWrapper, SizedAliasWrapper>();
}

// NATIVE-DAG: return 0U;
// NATIVE-DAG: return 1U;
// NATIVE: computeMain
// WGSL-DAG: return u32(0);
// WGSL-DAG: return u32(1);
// WGSL: fn computeMain
// SPIRV: OpEntryPoint GLCompute
// METALLIB: define void @computeMain
// RESULT: 0
// RESULT-NEXT: 0
// RESULT-NEXT: 0
// RESULT-NEXT: 0
// RESULT-NEXT: 0
// RESULT-NEXT: 0
// RESULT-NEXT: 0
// RESULT-NEXT: 0
// RESULT-NEXT: 1
// RESULT-NEXT: 1
// RESULT-NEXT: 1
// RESULT-NEXT: 1
// RESULT-NEXT: 1
// RESULT-NEXT: 1
// RESULT-NEXT: 1
// RESULT-NEXT: 1
// RESULT-NEXT: 0
// RESULT-NEXT: 0
// RESULT-NEXT: 1
// RESULT-NEXT: 1
