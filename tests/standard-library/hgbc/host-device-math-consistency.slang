// TEST:INTERPRET(hgvm): -api vulkan
// TEST:INTERPRET(hgvm): -api cuda
// TEST:INTERPRET(hgvm): -api d3d12

// Evaluates each host-lowered operation once in HGVM host code and once in a GPU dispatch with the
// same edge-case inputs, then compares the results.
//
// Consider `int(asfloat(0x7fc00000u))`: HGVM converts NaN to zero, and so must every GPU backend,
// or host code and device code would disagree about the same expression. Integer-valued results
// must therefore match exactly. Float-valued results may differ in two documented ways: any NaN
// matches any NaN, because GPUs do not preserve NaN payloads through arithmetic, and subnormal
// inputs or results may be flushed to zero, as D3D12 and most Vulkan drivers do. Approximate math
// such as `rsqrt` may additionally differ by a few units in the last place.
//
// The CUDA variant is a known failure listed in tests/expected-failure-h-slang.txt: Slang's CUDA
// output does not mask shift amounts, so cases 0, 1, 2, and 28 differ for amounts of 32 or more.

import slang.compute.buffer_utils;
import slang.runtime;

static const uint kInputCount = 20;
static const uint kCaseCount = 32;
static const uint kNoMismatch = 0xffffffffu;

static const uint kInputs[kInputCount] = {
    0u,
    1u,          // smallest subnormal float
    31u,
    32u,
    33u,
    0xffffffffu, // -1 as int, NaN as float
    0x80000000u, // INT_MIN, -0.0
    0x7fffffffu,
    0x3f800000u, // 1.0
    0xbfc00000u, // -1.5
    0x4f32d05eu, // 3e9
    0xcf32d05eu, // -3e9
    0x7f800000u, // +inf
    0xff800000u, // -inf
    0x7fc00000u, // quiet NaN
    0x7fa00001u, // NaN with a payload
    0x40200000u, // 2.5
    0x477fe000u, // 65504, the largest half
    0x47800000u, // 65536, beyond half
    0xc0490fdbu, // -pi
};

/// Describes how one case's host and device results are compared.
enum class ResultKind
{
    Integer,
    ExactFloat,
    ApproximateFloat,
}

ResultKind getResultKind(uint caseIndex)
{
    switch (caseIndex)
    {
    case 9:
    case 12:
    case 13:
    case 14:
    case 18:
    case 19:
    case 20:
    case 29:
    case 30:
        return ResultKind.ExactFloat;
    case 21:
    case 22:
    case 23:
    case 24:
    case 25:
    case 26:
    case 27:
        return ResultKind.ApproximateFloat;
    default:
        return ResultKind.Integer;
    }
}

/// Evaluates one operation on two input bit patterns and returns the result's bits.
[noinline]
uint evaluate(uint caseIndex, uint a, uint b)
{
    float fa = asfloat(a);
    float fb = asfloat(b);
    int ia = asint(a);
    int ib = asint(b);
    switch (caseIndex)
    {
    case 0:
        return asuint(ia >> ib);
    case 1:
        return a >> b;
    case 2:
        return a << b;
    case 3:
        return a & b;
    case 4:
        return a | b;
    case 5:
        return a ^ b;
    case 6:
        return ~a;
    case 7:
        return 0u - a;
    case 8:
        return asuint(abs(ia));
    case 9:
        return asuint(abs(fa));
    case 10:
        return asuint(int(fa));
    case 11:
        return uint(fa);
    case 12:
        return asuint(float(a));
    case 13:
        return asuint(float(ia));
    case 14:
        // `float(half(x))` is covered by tests/hgbc/host-integer-bitcast-math.slang instead:
        // the NVIDIA Vulkan driver removes the f32-to-f16-to-f32 round trip that SPIR-V requests.
        return asuint(max(fa, fb));
    case 15:
        return isnan(fa) ? 1u : 0u;
    case 16:
        return isinf(fa) ? 1u : 0u;
    case 17:
        return isfinite(fa) ? 1u : 0u;
    case 18:
        return asuint(asfloat(a ^ b) * 1.0);
    case 19:
        return asuint(clamp(fa, -1.0, 1.0));
    case 20:
        return asuint(saturate(fa));
    case 21:
        return asuint(frac(fa));
    case 22:
        return asuint(rsqrt(fa));
    case 23:
        return asuint(rcp(fa));
    case 24:
        return asuint(length(float3(fa, fb, 1.0)));
    case 25:
        return asuint(normalize(float3(fa, fb, 1.0)).x);
    case 26:
        return asuint(cross(float3(fa, fb, 1.0), float3(fb, 1.0, fa)).y);
    case 27:
        return asuint(distance(float2(fa, fb), float2(1.0, -1.0)));
    case 28:
        return (uint3(a, b, a) >> uint3(3u, a, b)).y;
    case 29:
        return asuint(float3(uint3(a, b, 1u)).y);
    case 30:
        return asuint(abs(float2(fa, fb))).y;
    default:
        return asuint(int2(float2(fa, fb))).y;
    }
}

/// Reports whether float bits are zero or subnormal, which GPUs may flush to zero.
bool isZeroOrSubnormal(uint bits)
{
    return (bits & 0x7fffffffu) < 0x00800000u;
}

/// Compares a host result with a device result for one case.
bool resultsAgree(uint caseIndex, uint a, uint b, uint host, uint device)
{
    if (host == device)
        return true;
    ResultKind kind = getResultKind(caseIndex);
    if (kind == ResultKind.Integer)
        return false;
    if ((isZeroOrSubnormal(a) && a != 0u && a != 0x80000000u) ||
        (isZeroOrSubnormal(b) && b != 0u && b != 0x80000000u))
        return true;
    float h = asfloat(host);
    float d = asfloat(device);
    if (isnan(h) || isnan(d))
        return isnan(h) && isnan(d);
    if (isZeroOrSubnormal(host) && isZeroOrSubnormal(device))
        return true;
    if (kind == ResultKind.ExactFloat || isinf(h) || isinf(d))
        return false;
    float scale = max(1.0, max(abs(h), abs(d)));
    return abs(h - d) <= scale * 7.62939453125e-6;
}

[host]
void main()
{
    static const uint kPairCount = kInputCount * kInputCount;
    static const uint kTotal = kCaseCount * kPairCount;

    let inputValues = RuntimeArray<uint>::alloc(kInputCount);
    for (uint i = 0; i < kInputCount; ++i)
        inputValues[i] = kInputs[i];
    let inputs = RWStructuredBuffer<uint>::alloc(kInputCount);
    copy(inputs, 0, inputValues, 0, kInputCount);

    let hostValues = RuntimeArray<uint>::alloc(kTotal);
    for (uint i = 0; i < kTotal; ++i)
    {
        uint caseIndex = i / kPairCount;
        uint a = inputValues[(i / kInputCount) % kInputCount];
        uint b = inputValues[i % kInputCount];
        hostValues[i] = evaluate(caseIndex, a, b);
    }
    let expected = RWStructuredBuffer<uint>::alloc(kTotal);
    copy(expected, 0, hostValues, 0, kTotal);

    let outputs = RWStructuredBuffer<uint>::alloc(kTotal);
    Grid(kTotal).dispatch(
        (uint i) =>
        {
            outputs[i] = evaluate(
                i / kPairCount,
                inputs[(i / kInputCount) % kInputCount],
                inputs[i % kInputCount]);
        });

    // Per case: the number of mismatches and the lowest mismatching index.
    let mismatchCounts = RWStructuredBuffer<uint>::alloc(kCaseCount);
    let firstMismatches = RWStructuredBuffer<uint>::alloc(kCaseCount);
    mismatchCounts.fill(0u);
    firstMismatches.fill(kNoMismatch);
    Grid(kTotal).dispatch(
        (uint i) =>
        {
            uint caseIndex = i / kPairCount;
            uint a = inputs[(i / kInputCount) % kInputCount];
            uint b = inputs[i % kInputCount];
            if (!resultsAgree(caseIndex, a, b, expected[i], outputs[i]))
            {
                InterlockedAdd(mismatchCounts[caseIndex], 1u);
                InterlockedMin(firstMismatches[caseIndex], i);
            }
        });

    uint totalMismatches = 0;
    for (uint caseIndex = 0; caseIndex < kCaseCount; ++caseIndex)
    {
        uint count = readBack(mismatchCounts, caseIndex);
        if (count == 0)
            continue;
        totalMismatches += count;
        uint i = readBack(firstMismatches, caseIndex);
        print(format(
            "case {}: {} mismatches; first a {} b {} host {} device {}",
            caseIndex,
            count,
            inputValues[(i / kInputCount) % kInputCount],
            inputValues[i % kInputCount],
            hostValues[i],
            readBack(outputs, i)));
    }
    assert(totalMismatches == 0);
}
