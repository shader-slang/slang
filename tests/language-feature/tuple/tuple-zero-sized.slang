// Tests for working with zero-sized tuples. Since these mostly evaporate at
// runtime, the test here is just making sure the compiler doesn't crash when
// handling them.
//
// We do also check the comparison operators.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj

//TEST_INPUT:ubuffer(data=[1 2 3 4], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

Tuple<> zstGlobalNoInitializer;

Tuple<> zstGlobalWithInitializer0 = makeTuple();
Tuple<> zstGlobalWithInitializer1 = Tuple<>();

Tuple<> returnsZst0() {
    return Tuple<>();
}

Tuple<> returnsZst1() {
    return makeTuple();
}

bool compareZst() {
    return returnsZst0() == returnsZst1()
        && zstGlobalWithInitializer1 == zstGlobalWithInitializer0
        ;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // CHECK: 1
    outputBuffer[0] = compareZst() ? 1 : 0;
}
