// ref-accessor-unannotated.slang
//
// An unannotated property `ref` accessor is implicitly mutating (like `set`), so
// its implicit `this` is passed by reference: the reference the accessor returns
// denotes the caller's storage, and a read-modify-write through it updates that
// storage. This is the unannotated `ref` form from #9636.
// https://github.com/shader-slang/slang/issues/9636, #12487

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-metal -output-using-type

struct Container
{
    int _v;
    property int value
    {
        get { return _v; }
        ref { return _v; }
    }
}

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    Container obj;
    obj._v = 4;

    // A read-modify-write through the unannotated `ref` updates the caller's storage.
    obj.value++;
    // CHECK: 5
    outputBuffer[0] = obj.value;

    // A plain field read observes the update the accessor made to `this`.
    // CHECK-NEXT: 5
    outputBuffer[1] = obj._v;
}
