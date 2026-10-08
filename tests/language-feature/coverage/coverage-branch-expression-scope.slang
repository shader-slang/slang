// Verifies the expression-level branch coverage scope: scalar `?:`, `&&`,
// and `||` are branch sites with a true and a false arm, while forms that
// do not branch get no counters. Vector `?:` and vector `&&` evaluate every
// operand without short-circuiting, and an expression lowered into a
// global's initializer is outside any function body.

//TEST:SIMPLE(filecheck=CHECK):-target cpp -stage compute -entry computeMain -trace-branch-coverage

RWStructuredBuffer<uint> outputBuffer;

static bool gFlag = (outputBuffer[1] != 0u) && (outputBuffer[2] != 0u);
static const uint kPicked = (3u > 2u) ? 5u : 6u;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint value = tid.x;
    if ((tid.x != 0u) && (tid.y != 0u))
        value += 1u;
    else
        value += 2u;

    value += (tid.z != 0u) ? 3u : 4u;
    value += ((tid.x == 1u) || (tid.y == 1u)) ? 0u : 5u;

    bool3 lanes = tid != uint3(0u, 0u, 0u);
    uint3 picked = lanes ? uint3(1u, 2u, 3u) : uint3(4u, 5u, 6u);
    bool3 both = lanes && (tid != uint3(1u, 1u, 1u));
    value += picked.x + uint(both.y) + uint(gFlag) + kPicked;
    outputBuffer[0] = value;
}

// Five evaluated predicate sites with two arms each: both operands of
// the if condition, the scalar ternary condition, and both operands of
// the logical ternary condition. Each evaluated condition also records a
// line event, and so does a ternary arm or right operand that begins on a
// different line than its condition: thirteen slots in total. Vector and
// global expressions add none.
//CHECK-DAG: _slang_atomic_add_u64{{.*}}_slang_coverage{{.*int\(0\)}}
//CHECK-DAG: _slang_atomic_add_u64{{.*}}_slang_coverage{{.*int\(1\)}}
//CHECK-DAG: _slang_atomic_add_u64{{.*}}_slang_coverage{{.*int\(2\)}}
//CHECK-DAG: _slang_atomic_add_u64{{.*}}_slang_coverage{{.*int\(3\)}}
//CHECK-DAG: _slang_atomic_add_u64{{.*}}_slang_coverage{{.*int\(4\)}}
//CHECK-DAG: _slang_atomic_add_u64{{.*}}_slang_coverage{{.*int\(5\)}}
//CHECK-DAG: _slang_atomic_add_u64{{.*}}_slang_coverage{{.*int\(6\)}}
//CHECK-DAG: _slang_atomic_add_u64{{.*}}_slang_coverage{{.*int\(7\)}}
//CHECK-DAG: _slang_atomic_add_u64{{.*}}_slang_coverage{{.*int\(8\)}}
//CHECK-DAG: _slang_atomic_add_u64{{.*}}_slang_coverage{{.*int\(9\)}}
//CHECK-DAG: _slang_atomic_add_u64{{.*}}_slang_coverage{{.*int\(10\)}}
//CHECK-DAG: _slang_atomic_add_u64{{.*}}_slang_coverage{{.*int\(11\)}}
//CHECK-DAG: _slang_atomic_add_u64{{.*}}_slang_coverage{{.*int\(12\)}}
//CHECK-NOT: _slang_atomic_add_u64{{.*}}_slang_coverage{{.*int\(13\)}}
