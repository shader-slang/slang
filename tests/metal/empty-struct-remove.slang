//TEST:SIMPLE(filecheck=LIB):-target metallib -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST(compute, metal):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-metal -compute -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -output-using-type

// METAL-NOT: struct emptyStruct
struct emptyStruct
{
    void set(RWStructuredBuffer<int> buffer, int value) {buffer[0] = value;}
}


struct MyStruct
{
    RWStructuredBuffer<int> buffer;
    int value;
    void set()
    {
        e.set(buffer, value);
    }
    emptyStruct e;
}

//TEST_INPUT: set param = new MyStruct{ out ubuffer(data=[0]), 5, {}}
ParameterBlock<MyStruct> param;

// LIB: @computeMain
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(
    uint tid: SV_DispatchThreadID,
)
{
    // BUF: 5
    param.set();
}
