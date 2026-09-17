// Boundary test for shader-slang/slang#13158: the enum->scalar common-type join
// does not apply to two enums (even with the same tag type), so `?:` over two
// distinct enums gains no common scalar and must not fall back to the deprecated
// vector overload — it produces a clean "no applicable overload" error.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-target spirv -entry computeMain -stage compute

enum Weekday : uint
{
    Mon = 1,
}

enum Planet : uint
{
    Mars = 2,
}

RWStructuredBuffer<uint> output;

[numthreads(1, 1, 1)]
void computeMain()
{
    bool cond = true;
    output[0] = cond ? Weekday::Mon : Planet::Mars;
}

//CHECK-NOT: 30056
//CHECK: E39999
//CHECK: no overload for '?:'
//CHECK-NOT: 30056
