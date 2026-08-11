// https://github.com/shader-slang/slang/issues/12265
// A 64-bit atomic through a helper on a `RWByteAddressBuffer` must not change the width or offset of
// an unrelated 32-bit atomic on that same buffer.
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -compute -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0], stride=4):out,name=buffer
RWByteAddressBuffer buffer;

// Passing `uav` by value forces the byte-address-buffer legalization to take its `IRParam` path for
// the 64-bit atomic; that must not leak `ulonglong` onto the 32-bit atomic in `computeMain`.
void doMax(RWByteAddressBuffer uav)
{
    uav.InterlockedMaxU64(0, 5);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    doMax(buffer);
    uint prev;
    buffer.InterlockedAdd(16, 3, prev);
}

// The 64-bit max lands at byte 0 (element 0 == 5); the 32-bit add lands at byte 16 (element 4 == 3).
// Element 8 (byte 32) must stay 0 — a value there would mean the add was miswidened to 64-bit.
// CHECK: 5
// CHECK-NEXT: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 3
// CHECK-NEXT: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 0
