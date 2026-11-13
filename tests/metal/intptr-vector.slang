//TEST:SIMPLE(filecheck=METAL): -target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -entry computeMain -stage compute

// Vectors of `intptr_t` / `uintptr_t` must be emitted as `long<N>` / `ulong<N>`, not as the
// scalar `long` / `ulong`, which Metal rejects ("excess elements in scalar initializer",
// "member reference base type 'ulong' is not a structure or union").

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // METAL-DAG: ulong2(uint64_t(2)
    // METAL-DAG: long3(
    // METAL-NOT: {{[^_a-z0-9]}}ulong(uint64_t(2)
    vector<uintptr_t, 2> c = vector<uintptr_t, 2>(2uz) + vector<uintptr_t, 2>(tid.xy);
    vector<intptr_t, 3> d = vector<intptr_t, 3>(tid) * 2;
    vector<intptr_t, 3> e = max(d, vector<intptr_t, 3>(intptr_t(tid.y)));
    outputBuffer[0] = int(c.y + uintptr_t(e.z));
    // METALLIB: @computeMain
}
