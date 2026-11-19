//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target wgsl -stage compute -entry computeMain

// Test that 8-bit literals kept at their own width reach WGSL literal emission and are
// rejected once per type. Only an 8-bit-typed buffer keeps the literal from widening to
// int and constant-folding, so these stores exercise the literal-emission rejection path.

//CHECK: 8-bit integers not supported in WGSL
//CHECK: 8-bit integer type 'uint8_t' is not supported by the WGSL backend.

//CHECK: 8-bit integers not supported in WGSL
//CHECK: 8-bit integer type 'int8_t' is not supported by the WGSL backend.

RWStructuredBuffer<uint8_t> u8Buf;
RWStructuredBuffer<int8_t> i8Buf;

[numthreads(1,1,1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    u8Buf[dispatchThreadID.x] = uint8_t(7);
    i8Buf[dispatchThreadID.y] = int8_t(7);

    // A second literal of each width must not produce a duplicate error.
    u8Buf[dispatchThreadID.z] = uint8_t(9);
    i8Buf[dispatchThreadID.x] = int8_t(9);
}
