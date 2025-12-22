//TEST:COMPILE(filecheck=CHECK-CUDA):-stage compute -entry computeMain -target cuda
//TEST:COMPILE(filecheck=CHECK-HLSL):-stage compute -entry computeMain -target hlsl


public struct MyStruct<T, uint N, uint SubgroupSize>
    where T : __BuiltinFloatingPointType
{
    internal static const uint A = sizeof(uint4) / sizeof(T);
    internal static const uint B = ((N + (A - 1)) / A) * A;
    internal T[B] data;
}

RWStructuredBuffer<half> outputBuffer;

[numthreads(1, 1, 1)]
[shader("compute")]
void computeMain()
{
    MyStruct<half, 4, 5> s;
    //CHECK-HLSL: half  data_0[int(8)]
    //CHECK-CUDA: FixedArray<__half, 8> data_0

    for (int i = 0; i < s.B; i++)
    {
        outputBuffer[i + 1] = s.data[i];
    }
}
