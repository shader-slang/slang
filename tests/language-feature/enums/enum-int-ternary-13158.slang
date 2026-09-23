// Regression for https://github.com/shader-slang/slang/issues/13158: an enum
// arm and an int arm of `?:` (and of `select`) join to a scalar via the enum's
// tag type, so the scalar `operator?:` is selected instead of the deprecated
// vector overload.

//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry computeMain -stage compute
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-cpu -output-using-type

// ColorChannelU has a `uint` tag; ColorChannelI uses the default `int` tag (the
// issue's exact shape), whose `int` result assigns to the `uint` buffer element.
enum ColorChannelU : uint
{
    Red = 5,
    Green = 6,
    Blue = 7,
}

enum ColorChannelI
{
    Cyan = 8,
    Magenta = 9,
}

// Overloads that report the exact static type of their argument, so the buffer
// distinguishes a `uint` result (2) from an `int` result (1) — a `vector<int,1>`
// result would not select either scalar overload.
uint typeTag(int v)
{
    return 1;
}
uint typeTag(uint v)
{
    return 2;
}

// The enum->tag join is gated to common-type inference, so it must not leak into
// the constraint solver: `where optional T == int` stays benign for an enum
// argument (the optional constraint is simply unsatisfied), rather than
// fabricating a conflicting `T == int`.
void acceptsOptionalIntEq<T>(T x) where optional T == int {}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0], stride=4):out,name=output
RWStructuredBuffer<uint> output;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    bool cond = tid.x == 0;
    int chan = 3;

    output[0] = cond ? ColorChannelU::Red : 0;
    output[1] = cond ? 0 : ColorChannelU::Green;
    output[2] = select(cond, ColorChannelU::Red, 0);
    output[3] = cond ? ColorChannelI::Cyan : 0;
    output[4] = select(cond, ColorChannelI::Magenta, 0);
    output[5] = cond ? ColorChannelU::Blue : chan;

    // Exact-type probes: uint-tagged results are `uint` (2), int-tagged are `int` (1).
    output[6] = typeTag(cond ? ColorChannelU::Red : 0);
    output[7] = typeTag(select(cond, ColorChannelU::Red, 0));
    output[8] = typeTag(cond ? ColorChannelI::Cyan : 0);

    acceptsOptionalIntEq(ColorChannelI::Cyan);
}

//CHECK-NOT: 30056
//CHECK-NOT: 30019
//CHECK: OpEntryPoint

//BUF: 5
//BUF-NEXT: 0
//BUF-NEXT: 5
//BUF-NEXT: 8
//BUF-NEXT: 9
//BUF-NEXT: 7
//BUF-NEXT: 2
//BUF-NEXT: 2
//BUF-NEXT: 1
