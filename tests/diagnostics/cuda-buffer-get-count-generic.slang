//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target cuda -stage compute -entry main -DBUFFER_TYPE=StructuredBuffer
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target cuda -stage compute -entry main -DBUFFER_TYPE=RWStructuredBuffer

BUFFER_TYPE<uint> buffer;
RWStructuredBuffer<uint> output;

uint count<T : IArray<uint>>(T value)
{
    return uint(value.getCount());
}

// An interface call only reveals its unsupported buffer operation after specialization.
// CHECK: error[E55204]:
// CHECK: intrinsic operation 'buffer dimensions' is not supported for the current target
[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    output[0] = count(buffer);
}
