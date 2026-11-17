// Debug information must not prevent a pure helper from being inlined into a
// global constant initializer. CUDA does not allow dynamic device initialization.
//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -g -line-directive-mode none
//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -line-directive-mode none

[ForceInline]
bool isSet<T : __EnumType>(T value, T flag)
{
    return value & flag;
}

enum class Flags : uint
{
    udim = 1,
    flip = 2,
};

struct Handle<uint Bits>
{
    static const Flags flags = Flags(Bits);
    static const bool enabled = isSet(flags, Flags.udim);
    static const bool flip = isSet(flags, Flags.flip);
};

RWStructuredBuffer<uint> outputBuffer;

// CUDA-NOT: __device__ static const bool
// CUDA: void computeMain(
// CUDA: = 2U;
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[tid.x] = uint(Handle<3>.enabled) + uint(Handle<3>.flip);
}
