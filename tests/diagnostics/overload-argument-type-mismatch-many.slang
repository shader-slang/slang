// Regression test for https://github.com/shader-slang/slang/issues/7857
//
// Companion to overload-argument-type-mismatch.slang. With more than
// `maxCandidatesToPrint` (10) distinct candidates, the candidate notes are
// capped and a trailing "N more overload candidates" note (E40015) reports the
// remainder. This pins the interaction between dedup, the cap, and the
// `remainingCount` accounting.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct Test {}
struct Other {}

// Twelve distinct overloads, all of which fail to match because argument 0 is a
// `Test` and every candidate's first parameter is `Other`.
float many(Other x, float a) { return 0; }
float many(Other x, float2 a) { return 0; }
float many(Other x, float3 a) { return 0; }
float many(Other x, float4 a) { return 0; }
float many(Other x, int a) { return 0; }
float many(Other x, int2 a) { return 0; }
float many(Other x, int3 a) { return 0; }
float many(Other x, int4 a) { return 0; }
float many(Other x, uint a) { return 0; }
float many(Other x, uint2 a) { return 0; }
float many(Other x, uint3 a) { return 0; }
float many(Other x, uint4 a) { return 0; }

float test()
{
    return many(Test(), 0.0);
//CHECK: no overload for 'many' applicable to arguments of type (Test, float)
}

// The candidate notes appear in a deterministic order: equal-status candidates
// are ordered by source location (the sort's tie-breaker), i.e. declaration
// order. These ordered `CHECK`s pin that, so a regression that dropped the
// tie-breaker (making the order arbitrary across builds/platforms) would fail.
//CHECK: candidate: func many(Other, float) -> float
//CHECK: candidate: func many(Other, float2) -> float
//CHECK: candidate: func many(Other, float3) -> float
//CHECK: candidate: func many(Other, float4) -> float

// At most 10 candidate notes are printed, then the remaining 2 are summarized.
//CHECK: 2 more overload candidates
