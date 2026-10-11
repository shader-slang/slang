// A `groupshared` array and its elements are memory locations in the Workgroup storage class,
// so they can be passed to `__ref_readonly` and `__ref_writeonly` parameters.

//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry main -stage compute

// CHECK: OpTypePointer Workgroup

groupshared int gs[4];

int first(__ref_readonly int a[4])
{
    return a[0];
}

int readValue(__ref_readonly int x)
{
    return x;
}

void clear(__ref_writeonly int a[4])
{
    a[0] = 0;
    a[1] = 0;
    a[2] = 0;
    a[3] = 0;
}

void store(__ref_writeonly int x, int value)
{
    x = value;
}

RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void main(uint tid: SV_GroupIndex)
{
    if (tid == 0)
        clear(gs);
    GroupMemoryBarrierWithGroupSync();
    store(gs[tid], int(tid));
    GroupMemoryBarrierWithGroupSync();
    outputBuffer[tid] = first(gs) + readValue(gs[tid]);
}
