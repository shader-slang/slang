//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk

struct MyStruct
{
    half a;
    half b;
}



//TEST_INPUT:set inputBuffer = ubuffer(data=[1.0 3.0 4.0], stride=4)
RWStructuredBuffer<float> inputBuffer;


//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[shader("compute")]
void computeMain()
{
    vector<float, 1> a = {inputBuffer[0]};

    outputBuffer[0] = bit_cast<uint>(a);
    // BUF: 3F800000

    MyStruct s = MyStruct(half(inputBuffer[1]), half(inputBuffer[2]));
    outputBuffer[1] = bit_cast<uint>(s);
    // 2.0 : 0x4000
    // 3.0 : 0x4200
    // BUF-NEXT: 44004200

}
