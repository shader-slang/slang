//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -O3 -line-directive-mode none
//TEST:SIMPLE(filecheck=PTX): -target ptx -entry computeMain -stage compute -O3

// A literal record table is a legal static CUDA initializer. Keep one device table
// rather than materializing the complete aggregate for each invocation.
struct Inner
{
    uint a;
    uint b;
};

struct Record
{
    Inner inner;
    uint2 vector;
    uint values[2];
};

static const Record records[2] =
{
    { { 1, 2 }, uint2(3, 4), { 5, 6 } },
    { { 7, 8 }, uint2(9, 10), { 11, 12 } },
};

// A user-defined constructor is executable code, not a static aggregate initializer.
// It must remain invocation-local unless a separate constant-evaluation pass proves it.
struct DynamicRecord
{
    uint value;

    __init(uint input)
    {
        value = input + 1;
    }
};

static const DynamicRecord dynamicRecords[2] = { DynamicRecord(13), DynamicRecord(17) };

// CUDA: __device__ static const FixedArray<Record_{{[0-9]+}}, 2> records_{{[0-9]+}} =
// CUDA: Record_{{[0-9]+}}{Inner_{{[0-9]+}}{1U, 2U}
// CUDA-NOT: FixedArray<Record_{{[0-9]+}}, 2> records_{{[0-9]+}};
// CUDA: extern "C" __global__ void computeMain
// CUDA: FixedArray<DynamicRecord_{{[0-9]+}}, 2> dynamicRecords_{{[0-9]+}} =

RWStructuredBuffer<uint> output;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    const uint index = tid.x & 1;
    output[tid.x] = records[index].inner.a + records[index].inner.b +
                    records[index].vector[tid.y & 1] + records[index].values[tid.z & 1] +
                    dynamicRecords[index].value;
}

// PTX: .visible .entry
