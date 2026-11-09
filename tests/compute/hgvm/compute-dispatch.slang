//TEST:INTERPRET(hgvm,filecheck=CHECK): -api default

[shader("compute")]
[numthreads(1, 1, 1)]
void writeValue(RWStructuredBuffer<uint> output)
{
    output[0] = 42;
}

[host]
void main()
{
    RWStructuredBuffer<uint> output = RWStructuredBuffer<uint>::alloc(1);
    dispatch(writeValue, uint3(1, 1, 1))(output);
    print(readBack(output, 0));
}

// CHECK: 42
