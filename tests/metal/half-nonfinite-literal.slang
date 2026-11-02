//TEST:SIMPLE(filecheck=METAL): -target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -entry computeMain -stage compute

// A `half` infinity or NaN constant must be a `half` in the MSL. `(1.0 / 0.0)` alone is a
// `float`, so `bit_cast<uint16_t>` of it became `as_type<ushort>(float)`, which Metal rejects
// ("as_type cast from 'float' to 'ushort' is not allowed").

RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // METAL: as_type<ushort>(half(1.0 / 0.0))
    // METAL: as_type<ushort>(half(-1.0 / 0.0))
    // METAL: as_type<ushort>(half(0.0 / 0.0))
    outputBuffer[0] = uint(bit_cast<uint16_t>(half(1.0 / 0.0)));
    outputBuffer[1] = uint(bit_cast<uint16_t>(half(-1.0 / 0.0)));
    outputBuffer[2] = uint(bit_cast<uint16_t>(half(0.0 / 0.0)));
    // METALLIB: @computeMain
}
