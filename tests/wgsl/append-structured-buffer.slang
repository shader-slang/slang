//TEST:SIMPLE(filecheck=WGSL): -target wgsl -stage compute -entry computeMain

RWStructuredBuffer<int> inBuffer;

AppendStructuredBuffer<int> outputBuffer;

// WGSL atomic built-ins take a `ptr<AS, atomic<T>>`, so the append counter must
// be emitted with an atomic element type and appended to with atomicAdd.
// WGSL: array<atomic<i32>>
// WGSL: atomicAdd

[numthreads(4, 1, 1)]
void computeMain(uint i : SV_GroupIndex)
{
    outputBuffer.Append(inBuffer[i]);
}
