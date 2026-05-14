// byte-address-buffer-load.slang
//
// Verifies that all ByteAddressBuffer.Load variants compile to WGSL.
// The struct itself and the underlying __byteAddressBufferLoad intrinsic
// already include `wgsl` in their [require] capabilities; this test
// validates that the public Load/Load2/Load3/Load4 methods and their
// Aligned overloads are also correctly gated for WGSL.
//
// For WGSL targets, the byte-address legalization pass scalarizes vector
// loads (Load2/3/4) into multiple single-word loads, so each LoadN call
// produces N array accesses in the emitted WGSL.

//TEST:SIMPLE(filecheck=CHECK): -target wgsl -stage compute -entry computeMain

ByteAddressBuffer inputBuffer;
RWStructuredBuffer<uint> outputBuffer;

// Buffer declaration: read-only storage, backed by array<u32>.
// CHECK: array<u32>

[numthreads(1, 1, 1)]
void computeMain()
{
    uint idx = 0;

    // --- Single-word loads ---

    // ByteAddressBuffer.Load(int)
    // CHECK: inputBuffer{{.*}}[
    uint v1 = inputBuffer.Load(0);
    outputBuffer[idx++] = v1;

    // ByteAddressBuffer.Load<T> (generic — no [require], inherits from struct)
    // CHECK: inputBuffer{{.*}}[
    uint v2 = inputBuffer.Load<uint>(4);
    outputBuffer[idx++] = v2;

    // ByteAddressBuffer.LoadAligned<T>(uint) — generic, 1-arg
    // CHECK: inputBuffer{{.*}}[
    uint v3 = inputBuffer.LoadAligned<uint>(8);
    outputBuffer[idx++] = v3;

    // ByteAddressBuffer.LoadAligned<T>(uint, uint) — generic, 2-arg explicit alignment
    // CHECK: inputBuffer{{.*}}[
    uint v4 = inputBuffer.LoadAligned<uint>(12, 4);
    outputBuffer[idx++] = v4;

    // --- Multi-word loads (scalarized to N single-word loads for WGSL) ---

    // ByteAddressBuffer.Load2(uint)
    // CHECK: inputBuffer{{.*}}[
    uint2 v5 = inputBuffer.Load2(16);
    outputBuffer[idx++] = v5.x + v5.y;

    // ByteAddressBuffer.Load3(uint)
    // CHECK: inputBuffer{{.*}}[
    uint3 v6 = inputBuffer.Load3(24);
    outputBuffer[idx++] = v6.x + v6.y + v6.z;

    // ByteAddressBuffer.Load4(uint)
    // CHECK: inputBuffer{{.*}}[
    uint4 v7 = inputBuffer.Load4(36);
    outputBuffer[idx++] = v7.x + v7.y + v7.z + v7.w;

    // --- Aligned multi-word loads (1-arg overloads) ---

    // ByteAddressBuffer.Load2Aligned(uint)
    // CHECK: inputBuffer{{.*}}[
    uint2 v8 = inputBuffer.Load2Aligned(56);
    outputBuffer[idx++] = v8.x;

    // ByteAddressBuffer.Load3Aligned(uint)
    // CHECK: inputBuffer{{.*}}[
    uint3 v9 = inputBuffer.Load3Aligned(64);
    outputBuffer[idx++] = v9.x;

    // ByteAddressBuffer.Load4Aligned(uint)
    // CHECK: inputBuffer{{.*}}[
    uint4 v10 = inputBuffer.Load4Aligned(80);
    outputBuffer[idx++] = v10.x;

    // --- Aligned multi-word loads (2-arg overloads, explicit alignment) ---

    // ByteAddressBuffer.Load2Aligned(uint, uint)
    // CHECK: inputBuffer{{.*}}[
    uint2 v11 = inputBuffer.Load2Aligned(96, 8);
    outputBuffer[idx++] = v11.x;

    // ByteAddressBuffer.Load3Aligned(uint, uint)
    // CHECK: inputBuffer{{.*}}[
    uint3 v12 = inputBuffer.Load3Aligned(104, 4);
    outputBuffer[idx++] = v12.x;

    // ByteAddressBuffer.Load4Aligned(uint, uint)
    // CHECK: inputBuffer{{.*}}[
    uint4 v13 = inputBuffer.Load4Aligned(116, 4);
    outputBuffer[idx++] = v13.x;
}
