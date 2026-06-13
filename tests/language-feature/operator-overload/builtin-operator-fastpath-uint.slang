// Builtin-operator fast-path coverage for unsigned integer operands. The IR ops (`kIROp_IRem`,
// `kIROp_Rsh`) are signedness-agnostic; the emitter selects the unsigned form (e.g. SPIR-V
// OpUMod / OpShiftRightLogical) from the operand type. This test confirms unsigned operands are
// recognized as fast-path-eligible and round-trip with the correct unsigned semantics (notably a
// *logical* right shift on a value with the high bit set, and unsigned modulo of a large value).
// All operands are runtime values so nothing folds away.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-d3d12 -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Mixed-sign, same-size promotion: `int OP uint` promotes to `uint` (size-tie -> unsigned
    // wins, per getBuiltinArithmeticCommonType). The signed `-1` is therefore compared as
    // 0xFFFFFFFF, so `neg > zero` is true (it would be false under signed semantics).
    int neg = int(outputBuffer[8]) - 1; // -1
    uint zero = outputBuffer[9];         // 0
    outputBuffer[8] = (neg > zero) ? 111u : 0u; // unsigned compare -> 111
    outputBuffer[9] = uint(neg) + zero + 1u;     // 0xFFFFFFFF + 1 wraps -> 0

    uint big = outputBuffer[0] + 0xFFFFFFFFu; // 0xFFFFFFFF (4294967295)
    uint three = outputBuffer[1] + 3u;        // 3

    outputBuffer[0] = big >> 1;        // logical shift: 0x7FFFFFFF = 2147483647
    outputBuffer[1] = big % three;     // 4294967295 % 3 = 0
    outputBuffer[2] = big / three;     // 1431655765
    outputBuffer[3] = (big > three) ? 1u : 0u; // 1 (unsigned compare)

    // Unsigned vector: logical shift + modulo component-wise.
    uint4 v = uint4(big, three, big, three); // (BIG,3,BIG,3)
    uint4 sh = v >> uint4(1, 0, 4, 1);        // (0x7FFFFFFF, 3, 0x0FFFFFFF, 1)
    outputBuffer[4] = sh.x;            // 2147483647
    outputBuffer[5] = sh.z;            // 268435455 (0x0FFFFFFF)

    // vector op scalar broadcast (unsigned).
    uint4 vs = v + 1u;                 // (0,4,0,4)  (wraps: 0xFFFFFFFF+1 = 0)
    outputBuffer[6] = vs.x;            // 0
    outputBuffer[7] = vs.y;            // 4

    // CHECK: 2147483647
    // CHECK: 0
    // CHECK: 1431655765
    // CHECK: 1
    // CHECK: 2147483647
    // CHECK: 268435455
    // CHECK: 0
    // CHECK: 4
    // CHECK: 111
    // CHECK: 0
}
