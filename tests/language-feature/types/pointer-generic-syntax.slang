// `T*` and `Ptr<T>` are the same type; both can hold an address taken from a
// mutable structured-buffer element via __getAddress (shader-slang/slang#12581).

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-directly -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT:ubuffer(data=[5 15 25 35], stride=4):name=dataBuffer
RWStructuredBuffer<int> dataBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    int* ptrStar = __getAddress(dataBuffer[0]);
    Ptr<int> ptrGeneric = __getAddress(dataBuffer[2]);

    // CHECK: 5
    outputBuffer[0] = *ptrStar;

    // CHECK: 25
    outputBuffer[1] = *ptrGeneric;

    int* ptrFromGeneric = ptrGeneric;
    // CHECK: 25
    outputBuffer[2] = *ptrFromGeneric;

    Ptr<int> genericFromStar = ptrStar;
    // CHECK: 5
    outputBuffer[3] = *genericFromStar;
}
