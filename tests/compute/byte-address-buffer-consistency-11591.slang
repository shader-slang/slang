// byte-address-buffer-consistency-11591.slang

// Feature test for https://github.com/shader-slang/slang/issues/11545 — Slice 2 (#11591).
//
// The wide-vs-scalarized lowering decision is driven by the promised `alignment` alone, so a
// constant `location` and a runtime `location` with the same `alignment` lower identically. Before
// this change the constant-folded case used the actual offset and could wide-load a location that
// a runtime location with the same promise would have scalarized.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute

RWByteAddressBuffer buffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // Promise 8 does not cover the whole 16-byte `float4`, so the access is scalarized — even
    // though the constant location 16 is itself 16-aligned. (Before #11545 point 3 this
    // constant-folded case used the actual offset and emitted one wide `float4` load.)
    // CHECK-COUNT-4: Load<float >
    float4 a = buffer.LoadAligned<float4>(16, 8);

    // A runtime location with the same promise lowers the same way — four scalar loads.
    // CHECK-COUNT-4: Load<float >
    float4 b = buffer.LoadAligned<float4>(tid.x * 16, 8);

    buffer.Store<float4>(0, a + b, 16);
}
