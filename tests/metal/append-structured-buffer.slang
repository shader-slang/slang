//TEST:SIMPLE(filecheck=METAL): -target metal -stage compute -entry computeMain

RWStructuredBuffer<int> inBuffer;

AppendStructuredBuffer<int> outputBuffer;

// Metal implements the append counter by reinterpret-casting the counter
// pointer to atomic_int and using atomic_fetch_add.
// METAL: atomic_fetch_add_explicit
// METAL: atomic_int

[numthreads(4, 1, 1)]
void computeMain(uint i : SV_GroupIndex)
{
    outputBuffer.Append(inBuffer[i]);
}
