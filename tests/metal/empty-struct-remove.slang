
//TEST:SIMPLE(filecheck=LIB):-target metallib -entry computeMain -stage compute -DMETAL
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute -DMETAL

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

ParameterBlock<MyStruct> param;

// LIB: @computeMain
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(
    uint tid: SV_DispatchThreadID,
)
{
    param.set();
}
