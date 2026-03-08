// RWByteAddressBuffer.Load<IFoo> and .Store<IFoo> for existential types.
//
// ByteAddressBuffer provides raw byte-level access, and Load<T>/Store<T>
// with an interface type T should pack/unpack the existential tuple
// (RTTI + WitnessTableID + AnyValue) at the given byte offset.
//
// BUG (#10376): This currently crashes with ICE 99999 on SPIRV
// ("Unhandled local inst in spirv-emit: lookupWitness") and HLSL
// ("unexpected IR opcode during code emit"). On CPU the shader runs
// but produces empty output. Disabled until the emit stage handles
// existential types in ByteAddressBuffer Load/Store.

//DISABLED_TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type -cpu
//DISABLED_TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//DISABLED_TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type -emit-spirv-directly

[anyValueSize(8)]
interface IFoo
{
    float getValue();
}

struct ImplA : IFoo
{
    float v;
    float getValue() { return v; }
}

struct ImplB : IFoo
{
    float v;
    float getValue() { return v * 2.0; }
}

//TEST_INPUT: type_conformance ImplA:IFoo = 1
//TEST_INPUT: type_conformance ImplB:IFoo = 2

// Each existential element = 24 bytes (RTTI:8 + WitnessTableID:8 + AnyValue:8)
// Allocate space for 2 elements = 48 bytes = 12 uint32s
//TEST_INPUT: set byteBuffer = ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0], stride=4)
RWByteAddressBuffer byteBuffer;

//TEST_INPUT: ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    ImplA a;
    a.v = 7.0;
    IFoo fooA = a;
    byteBuffer.Store<IFoo>(0, fooA);

    ImplB b;
    b.v = 3.0;
    IFoo fooB = b;
    byteBuffer.Store<IFoo>(24, fooB);

    IFoo loaded0 = byteBuffer.Load<IFoo>(0);
    IFoo loaded1 = byteBuffer.Load<IFoo>(24);

    outputBuffer[0] = loaded0.getValue();  // ImplA: 7.0
    outputBuffer[1] = loaded1.getValue();  // ImplB: 3.0 * 2.0 = 6.0

    // CHECK: 7.0
    // CHECK: 6.0
}
