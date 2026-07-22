// cast-zero-to-struct.slang

// Test that HLSL legacy syntax for casting from literal zero to a
// struct is enabled in Slang 2026 and disabled in Slang 2027.
//
// In Slang 2026, cast from literal zero is the same as invoking the
// default initializer.
//
// In Slang 2027, cast from literal zero has no special meaning. It is
// the same as conversion from value zero. That is, the same as
// invocation of __init(0).

// 2025 - no warning about (S)0
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK_2025_2026): -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK_2025_2026):-cpu -shaderobj -output-using-type

// 2026 - warn about (S)0  (semantics change in 2027)
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK_2025_2026): -shaderobj -output-using-type -xslang -Wno-30087 -xslang -DSLANG2026=1
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK_2025_2026):-cpu -shaderobj -output-using-type -xslang -Wno-30087 -xslang -DSLANG2026=1

// 2027 - verify that the semantics changed
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK2027): -shaderobj -output-using-type -xslang -DSLANG2027=1
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK2027):-cpu -shaderobj -output-using-type -xslang -DSLANG2027=1

//TEST:SIMPLE(filecheck=WARNCHECK2025): -no-codegen -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=WARNCHECK2026): -no-codegen -stage compute -entry computeMain -DSLANG2026=1
//TEST:SIMPLE(filecheck=WARNCHECK2027): -no-codegen -stage compute -entry computeMain -DSLANG2027=1

#if defined(SLANG2027)
#language 2027
#elif defined(SLANG2026)
#language 2026
#else
#language 2025
#endif

struct S
{
    int2 a = { 1, 2 };
    int2 b;
    int2 c;
}

S test(int val)
{
    if (val == 0)
    {
        S s = (S)0; // triggers warning E30087 on Slang 2026
        return s;
    }
    else
    {
        S s = (S)val;
        return s;
    }
}

//TEST_INPUT: ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=gOutputBuffer
RWStructuredBuffer<int> gOutputBuffer;

[numthreads(2, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    S s = test(tid.x);
    gOutputBuffer[6U * tid.x + 0U] = s.a.x;
    gOutputBuffer[6U * tid.x + 1U] = s.a.y;
    gOutputBuffer[6U * tid.x + 2U] = s.b.x;
    gOutputBuffer[6U * tid.x + 3U] = s.b.y;
    gOutputBuffer[6U * tid.x + 4U] = s.c.x;
    gOutputBuffer[6U * tid.x + 5U] = s.c.y;
}

//CHECK_2025_2026: int32_t
//CHECK_2025_2026-NEXT: 1
//CHECK_2025_2026-NEXT: 2
//CHECK_2025_2026-NEXT: 0
//CHECK_2025_2026-NEXT: 0
//CHECK_2025_2026-NEXT: 0
//CHECK_2025_2026-NEXT: 0
//CHECK_2025_2026-NEXT: 1
//CHECK_2025_2026-NEXT: 1
//CHECK_2025_2026-NEXT: 0
//CHECK_2025_2026-NEXT: 0
//CHECK_2025_2026-NEXT: 0
//CHECK_2025_2026-NEXT: 0

//CHECK2027: int32_t
//CHECK2027-NEXT: 0
//CHECK2027-NEXT: 0
//CHECK2027-NEXT: 0
//CHECK2027-NEXT: 0
//CHECK2027-NEXT: 0
//CHECK2027-NEXT: 0
//CHECK2027-NEXT: 1
//CHECK2027-NEXT: 1
//CHECK2027-NEXT: 0
//CHECK2027-NEXT: 0
//CHECK2027-NEXT: 0
//CHECK2027-NEXT: 0

//WARNCHECK2025: result code = 0
//WARNCHECK2025-NOT: warning[E30087]
//WARNCHECK2025: standard output = {

//WARNCHECK2026: result code = 0
//WARNCHECK2026: warning[E30087]
//WARNCHECK2026: standard output = {

//WARNCHECK2027: result code = 0
//WARNCHECK2027-NOT: warning[E30087]
//WARNCHECK2027: standard output = {
