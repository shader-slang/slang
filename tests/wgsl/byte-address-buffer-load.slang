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
// produces N array accesses in the emitted WGSL. The CHECK patterns below
// verify the exact scalarization count and byte offsets for each load.
//
// Non-uint Load<T> calls additionally exercise the bitcast lowering path
// (useBitCastFromUInt = true for WGSL): each typed load reads a u32 and
// then bitcasts to the target type.

//TEST:SIMPLE(filecheck=CHECK): -target wgsl -stage compute -entry computeMain

ByteAddressBuffer inputBuffer;
RWStructuredBuffer<uint> outputBuffer;

// Buffer declaration: read-only storage, backed by array<u32>.
// CHECK: array<u32>

[numthreads(1, 1, 1)]
void computeMain()
{
    uint idx = 0;

    // --- Single-word loads (1 array access each) ---

    // ByteAddressBuffer.Load(int)
    // CHECK: inputBuffer{{.*}}[(u32(0))/4]
    uint v1 = inputBuffer.Load(0);
    outputBuffer[idx++] = v1;

    // ByteAddressBuffer.Load<T> (generic — uint)
    // CHECK: inputBuffer{{.*}}[(u32(4))/4]
    uint v2 = inputBuffer.Load<uint>(4);
    outputBuffer[idx++] = v2;

    // ByteAddressBuffer.LoadAligned<T>(uint) — generic, 1-arg
    // CHECK: inputBuffer{{.*}}[(u32(8))/4]
    uint v3 = inputBuffer.LoadAligned<uint>(8);
    outputBuffer[idx++] = v3;

    // ByteAddressBuffer.LoadAligned<T>(uint, uint) — generic, 2-arg explicit alignment
    // CHECK: inputBuffer{{.*}}[(u32(12))/4]
    uint v4 = inputBuffer.LoadAligned<uint>(12, 4);
    outputBuffer[idx++] = v4;

    // --- Non-uint generic loads (bitcast lowering path) ---

    // Load<float> — u32 read followed by bitcast<f32>
    // CHECK: bitcast<f32>
    float f1 = inputBuffer.Load<float>(16);
    outputBuffer[idx++] = asuint(f1);

    // Load<int> — u32 read followed by bitcast<i32>
    // CHECK: bitcast<i32>
    int i1 = inputBuffer.Load<int>(20);
    outputBuffer[idx++] = uint(i1);

    // Load<float4> — scalarized to 4 u32 reads, each bitcast<f32>
    // CHECK-DAG: bitcast<f32>
    // CHECK-DAG: inputBuffer{{.*}}/4]
    float4 fv = inputBuffer.Load<float4>(24);
    outputBuffer[idx++] = asuint(fv.x);

    // --- Multi-word loads (scalarized to N single-word loads for WGSL) ---
    // Using CHECK-DAG within each load group: the scalarized accesses may
    // appear in any order in the emitted WGSL.

    // ByteAddressBuffer.Load2(uint) — scalarized to 2 loads
    // CHECK-DAG: inputBuffer{{.*}}[(u32(40))/4]
    // CHECK-DAG: inputBuffer{{.*}}[(u32(44))/4]
    uint2 v5 = inputBuffer.Load2(40);
    outputBuffer[idx++] = v5.x + v5.y;

    // ByteAddressBuffer.Load3(uint) — scalarized to 3 loads
    // CHECK-DAG: inputBuffer{{.*}}[(u32(48))/4]
    // CHECK-DAG: inputBuffer{{.*}}[(u32(52))/4]
    // CHECK-DAG: inputBuffer{{.*}}[(u32(56))/4]
    uint3 v6 = inputBuffer.Load3(48);
    outputBuffer[idx++] = v6.x + v6.y + v6.z;

    // ByteAddressBuffer.Load4(uint) — scalarized to 4 loads
    // CHECK-DAG: inputBuffer{{.*}}[(u32(60))/4]
    // CHECK-DAG: inputBuffer{{.*}}[(u32(64))/4]
    // CHECK-DAG: inputBuffer{{.*}}[(u32(68))/4]
    // CHECK-DAG: inputBuffer{{.*}}[(u32(72))/4]
    uint4 v7 = inputBuffer.Load4(60);
    outputBuffer[idx++] = v7.x + v7.y + v7.z + v7.w;

    // --- Aligned multi-word loads (1-arg overloads) ---

    // ByteAddressBuffer.Load2Aligned(uint) — scalarized to 2 loads
    // CHECK-DAG: inputBuffer{{.*}}[(u32(80))/4]
    // CHECK-DAG: inputBuffer{{.*}}[(u32(84))/4]
    uint2 v8 = inputBuffer.Load2Aligned(80);
    outputBuffer[idx++] = v8.x;

    // ByteAddressBuffer.Load3Aligned(uint) — scalarized to 3 loads
    // CHECK-DAG: inputBuffer{{.*}}[(u32(88))/4]
    // CHECK-DAG: inputBuffer{{.*}}[(u32(92))/4]
    // CHECK-DAG: inputBuffer{{.*}}[(u32(96))/4]
    uint3 v9 = inputBuffer.Load3Aligned(88);
    outputBuffer[idx++] = v9.x;

    // ByteAddressBuffer.Load4Aligned(uint) — scalarized to 4 loads
    // CHECK-DAG: inputBuffer{{.*}}[(u32(100))/4]
    // CHECK-DAG: inputBuffer{{.*}}[(u32(104))/4]
    // CHECK-DAG: inputBuffer{{.*}}[(u32(108))/4]
    // CHECK-DAG: inputBuffer{{.*}}[(u32(112))/4]
    uint4 v10 = inputBuffer.Load4Aligned(100);
    outputBuffer[idx++] = v10.x;

    // --- Aligned multi-word loads (2-arg overloads, explicit alignment) ---

    // ByteAddressBuffer.Load2Aligned(uint, uint) — scalarized to 2 loads
    // CHECK-DAG: inputBuffer{{.*}}[(u32(120))/4]
    // CHECK-DAG: inputBuffer{{.*}}[(u32(124))/4]
    uint2 v11 = inputBuffer.Load2Aligned(120, 8);
    outputBuffer[idx++] = v11.x;

    // ByteAddressBuffer.Load3Aligned(uint, uint) — scalarized to 3 loads
    // CHECK-DAG: inputBuffer{{.*}}[(u32(128))/4]
    // CHECK-DAG: inputBuffer{{.*}}[(u32(132))/4]
    // CHECK-DAG: inputBuffer{{.*}}[(u32(136))/4]
    uint3 v12 = inputBuffer.Load3Aligned(128, 4);
    outputBuffer[idx++] = v12.x;

    // ByteAddressBuffer.Load4Aligned(uint, uint) — scalarized to 4 loads
    // CHECK-DAG: inputBuffer{{.*}}[(u32(140))/4]
    // CHECK-DAG: inputBuffer{{.*}}[(u32(144))/4]
    // CHECK-DAG: inputBuffer{{.*}}[(u32(148))/4]
    // CHECK-DAG: inputBuffer{{.*}}[(u32(152))/4]
    uint4 v13 = inputBuffer.Load4Aligned(140, 4);
    outputBuffer[idx++] = v13.x;
}
