// Test that arithmetic on a texel loaded from a normalized-format texture uses
// floating-point opcodes.
//
// A `RWTexture2D<unorm float>` load produces a value whose IR type is an
// AttributedType wrapping Float, so a classifier that switches on the outer type
// alone sees neither Float nor Bool and falls back to the integer opcode, which
// spirv-val rejects against a %float result type.
//
// Two constraints keep this test from passing without the fix. The element type
// must be scalar, because `unorm float2` and `unorm float4` already emit float
// opcodes. And each operation must consume a texel-derived value: the attribute
// survives a copy into a local, but not a result already computed from one, so
// folding the operands together would emit float opcodes either way.

// The NEG prefix is a second pass whose directives are all CHECK-NOT, so it scans the whole
// module. Trailing CHECK-NOTs in the positive pass would only cover the span after the last
// CHECK-DAG match.

//TEST:SIMPLE(filecheck=CHECK-SPIRV):-target spirv -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=NEG):-target spirv -entry computeMain -stage compute

RWTexture2D<unorm float> u1;
RWTexture2D<snorm float> s1;
RWStructuredBuffer<float> outBuf;

// One classifier feeds every opcode below, so all eleven operators are covered.
// CHECK-SPIRV-DAG: OpFAdd %float
// CHECK-SPIRV-DAG: OpFSub %float
// CHECK-SPIRV-DAG: OpFMul %float
// CHECK-SPIRV-DAG: OpFDiv %float
// CHECK-SPIRV-DAG: OpFNegate %float
// CHECK-SPIRV-DAG: OpFOrdLessThan %bool
// CHECK-SPIRV-DAG: OpFOrdLessThanEqual %bool
// CHECK-SPIRV-DAG: OpFOrdEqual %bool
// CHECK-SPIRV-DAG: OpFUnordNotEqual %bool
// CHECK-SPIRV-DAG: OpFOrdGreaterThanEqual %bool
// CHECK-SPIRV-DAG: OpFOrdGreaterThan %bool

// NEG-NOT: OpIAdd %float
// NEG-NOT: OpISub %float
// NEG-NOT: OpIMul %float
// NEG-NOT: OpUDiv %float
// NEG-NOT: OpSNegate %float
// NEG-NOT: OpULessThan %bool
// NEG-NOT: OpULessThanEqual %bool
// NEG-NOT: OpIEqual %bool
// NEG-NOT: OpINotEqual %bool
// NEG-NOT: OpUGreaterThanEqual %bool
// NEG-NOT: OpUGreaterThan %bool

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint2 p = tid.xy;

    // The divisor is loaded rather than a literal so that the division survives
    // as one: a float division by a constant is strength-reduced to a multiply,
    // which would leave no OpFDiv to assert once the classification is correct.
    float divisor = outBuf[9];

    outBuf[0] = u1[p] + 0.25;
    outBuf[1] = u1[p] - 0.25;
    outBuf[2] = u1[p] * 0.5;
    outBuf[3] = u1[p] / divisor;
    outBuf[4] = -s1[p];

    outBuf[5] = s1[p] < 0.5 ? 1.0 : 0.0;
    outBuf[6] = u1[p] <= divisor ? 1.0 : 0.0;
    outBuf[7] = u1[p] == divisor ? 1.0 : 0.0;
    outBuf[8] = u1[p] != divisor ? 1.0 : 0.0;
    outBuf[10] = u1[p] >= divisor ? 1.0 : 0.0;
    outBuf[11] = u1[p] > divisor ? 1.0 : 0.0;
}
