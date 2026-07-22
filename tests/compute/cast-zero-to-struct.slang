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


//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK2026): -shaderobj -output-using-type -xslang -Wno-30087
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK2026):-cpu -shaderobj -output-using-type -xslang -Wno-30087

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK2027): -shaderobj -output-using-type -xslang -DSLANG2027=1
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK2027):-cpu -shaderobj -output-using-type -xslang -DSLANG2027=1

#ifdef SLANG2027
#language 2027
#else
#language 2026
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
        S s = (S)0;
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

//CHECK2026: int32_t
//CHECK2026-NEXT: 1
//CHECK2026-NEXT: 2
//CHECK2026-NEXT: 0
//CHECK2026-NEXT: 0
//CHECK2026-NEXT: 0
//CHECK2026-NEXT: 0
//CHECK2026-NEXT: 1
//CHECK2026-NEXT: 1
//CHECK2026-NEXT: 0
//CHECK2026-NEXT: 0
//CHECK2026-NEXT: 0
//CHECK2026-NEXT: 0

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
