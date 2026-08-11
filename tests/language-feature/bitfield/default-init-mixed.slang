//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -compile-arg -msvc-style-bitfield-packing

// Regression for shader-slang/slang#11844: empty/partial initialization of a struct
// that mixes bitfields with a trailing normal field must zero-initialize the hidden
// bitfield backing storage. The defect is in front-end constructor synthesis and is
// therefore target-independent, so a -cpu run is sufficient (the local environment has
// no GPU); the second directive re-runs the same shader under MSVC bit-packing to cover
// that backing-layout path. The deterministic, backend-robust gates for this fix are the
// emit-level companion (default-init-mixed-emit.slang) and the E41021 diagnostic test
// (default-init-mixed-diagnostic.slang); this test additionally exercises the 32-bit,
// 64-bit (BaseType::UInt64) and two-backing-word lowering branches at runtime.

// CHECK: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 7
// CHECK-NEXT: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 0

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

// 32-bit backing word + a trailing normal field (the original repro).
struct Mixed32 { uint a : 10; uint b : 22; uint c; };
// 64-bit backing word (the BaseType::UInt64 branch) + a trailing normal field.
struct Mixed64 { uint64_t a : 40; uint64_t b : 20; uint c; };
// Two backing words: a normal field separates two bitfield runs.
struct TwoWord { uint x : 8; uint mid; uint y : 8; };

[numthreads(1, 1, 1)]
void computeMain()
{
    Mixed32 m0 = {};   // a, b, c all zero
    outputBuffer[0] = m0.a;
    outputBuffer[1] = m0.b;
    outputBuffer[2] = m0.c;

    Mixed32 m1 = {7};  // c == 7; a, b zero (backing word zero-initialized)
    outputBuffer[3] = m1.a;
    outputBuffer[4] = m1.b;
    outputBuffer[5] = m1.c;

    Mixed64 m2 = {};   // 64-bit backing word zero-initialized
    outputBuffer[6] = uint(m2.a);
    outputBuffer[7] = uint(m2.b);

    TwoWord m3 = {};   // both backing words zero-initialized
    outputBuffer[8] = m3.x;
    outputBuffer[9] = m3.y;
}
