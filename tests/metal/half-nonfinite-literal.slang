//TEST:SIMPLE(filecheck=METAL): -target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -entry computeMain -stage compute

// A `half` infinity or NaN constant must be a `half` in the MSL. `(1.0 / 0.0)` alone is a
// `float`, so `bit_cast<uint16_t>` of it became `as_type<ushort>(float)`, which Metal rejects
// ("as_type cast from 'float' to 'ushort' is not allowed").

RWStructuredBuffer<uint> outputBuffer;
RWStructuredBuffer<half> halfBuffer;

// The same constant in a module-scope `constant` array initializer (a run-time index keeps the
// table in memory) and as an argument of an overloaded call.
// METAL: constant array<half, int(3)> {{.*}} = { { half(1.0 / 0.0), half(-1.0 / 0.0), half(0.0 / 0.0) } };
static const half kNonFinite[3] = { half(1.0 / 0.0), half(-1.0 / 0.0), half(0.0 / 0.0) };

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // METAL: as_type<ushort>(half(1.0 / 0.0))
    // METAL: as_type<ushort>(half(-1.0 / 0.0))
    // METAL: as_type<ushort>(half(0.0 / 0.0))
    outputBuffer[0] = uint(bit_cast<uint16_t>(half(1.0 / 0.0)));
    outputBuffer[1] = uint(bit_cast<uint16_t>(half(-1.0 / 0.0)));
    outputBuffer[2] = uint(bit_cast<uint16_t>(half(0.0 / 0.0)));
    halfBuffer[0] = kNonFinite[tid.x];
    // METAL: min({{.*}}, half(1.0 / 0.0))
    halfBuffer[1] = min(halfBuffer[2], half(1.0 / 0.0));
    // METALLIB: @computeMain
}
