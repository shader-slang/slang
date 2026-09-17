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

// Overloads that report their argument's element type, so the buffer distinguishes
// a `uint` result (2) from an `int` result (1) — confirming a `uint`-tagged enum
// joins to `uint` and an `int`-tagged enum to `int`. Scalar-vs-vector is covered by
// this run's `CHECK-NOT: 30056` (selecting the deprecated vector `?:` overload would
// raise it); the element probe alone cannot, since a `vector<int,1>` would decay to
// `int` via the vector<T,1>->T conversion.
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

// The enum/scalar join serves all generic ordinary-argument inference, not only
// `?:`/`select`, so a user-defined `f<T>(T, T)` called with (enum, int) has a
// viable `T = int` candidate. This asserts that its viability does not flip or
// ambiguate overload selection: a more specific non-generic overload is
// preferred.
uint pickOverload<T>(T a, T b)
{
    return 100;
}
uint pickOverload(int a, int b)
{
    return 200;
}

// With no non-generic overload, the call is viable only when the enum/scalar
// join infers `T`. Returning `a` makes the result depend on that inference:
// `T = int`, so `ColorChannelI::Cyan` decays to 8.
T onlyGeneric<T>(T a, T b)
{
    return a;
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=output
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

    // Element-type probes: uint-tagged results are `uint` (2), int-tagged are `int` (1).
    output[6] = typeTag(cond ? ColorChannelU::Red : 0);
    output[7] = typeTag(select(cond, ColorChannelU::Red, 0));
    output[8] = typeTag(cond ? ColorChannelI::Cyan : 0);

    // The non-generic overload is preferred over the viable generic (no flip): 200.
    output[9] = pickOverload(ColorChannelI::Cyan, 0);
    // Generic-only: viable solely because the enum/scalar join infers T=int -> 8.
    output[10] = onlyGeneric(ColorChannelI::Cyan, 0);

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
//BUF-NEXT: 200
//BUF-NEXT: 8
