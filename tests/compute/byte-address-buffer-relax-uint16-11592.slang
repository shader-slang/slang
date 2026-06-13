// byte-address-buffer-relax-uint16-11592.slang

// Regression test for https://github.com/shader-slang/slang/issues/9958, via #11545 — Slice 3 (#11592).
//
// A power-of-two `alignment` smaller than the access type's aggregate size, but at least its
// scalar-component alignment, is now accepted (previously rejected by 41300). `uint16_t2` has a
// scalar alignment of 2, so `LoadAligned<uint16_t2>(location, 2)` must compile and scalarize into
// per-component loads rather than error.

// Checked on `hlsl` only: the CHECK matches the HLSL scalarized-load form `Load<uint16_t >`; on
// spirv the same access emits `OpLoad %ushort`, which would need a separate target-specific CHECK.
// The relaxation itself is target-independent (it runs in the byte-address legalizer, before emit).
//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute

RWByteAddressBuffer buffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // Alignment 2 == the scalar alignment of `uint16_t`: accepted (no E41300), and because it is
    // smaller than the 4-byte aggregate stride the access is scalarized into per-component loads.
    // CHECK-COUNT-2: Load<uint16_t >
    uint16_t2 v = buffer.LoadAligned<uint16_t2>(tid.x * 4, 2);
    buffer.Store<uint16_t2>(0, v);
}
