//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target cuda -stage compute -entry main -DBUFFER_TYPE=StructuredBuffer
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target cuda -stage compute -entry main -DBUFFER_TYPE=RWStructuredBuffer

BUFFER_TYPE<uint> buffer;
RWStructuredBuffer<uint> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    // CHECK: error[E36107]:
    // CHECK: buffer.getCount
    output[0] = buffer.getCount();
}
