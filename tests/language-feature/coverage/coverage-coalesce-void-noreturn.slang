// A `void` function that never returns must split its caller's region, the
// same as a value-returning one.
//
// The two reach that conclusion by different routes, which is why this needs
// its own test. Lowering gives a value-returning body that falls off the end
// an `IRMissingReturn`, which the exit analysis does not count as a normal
// exit. A `void` body instead gets an unconditional implicit `emitReturn()`
// (`slang-lower-to-ir.cpp`), so the concern is that a stray `IRReturn` left in
// an unreachable block would set `sawNormalExit` and suppress the split --
// the unsound direction, where the caller coalesces across a call that never
// comes back and reports the statements before it as never executed.
//
// It does not happen: a non-terminating `void` body reaches this pass with
// `unreachable` as its terminator and no `IRReturn` at all. This test pins
// that, since the analysis's correctness depends on it and the property lives
// in lowering rather than here.

//TEST:SIMPLE(filecheck=CHECK):-target cpp -stage compute -entry computeMain -trace-coverage

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<float> outputBuffer;

// `void`, and no reachable return.
void spin()
{
    for (;;)
    {
    }
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // One region up to and including the call...
    float a = (float)tid.x;
    spin();
    // ...and a second after it, because `spin` never returns.
    float c = a + 3.0f;
    outputBuffer[0] = c;
}

// `spin`'s own body, then the two regions of `computeMain`.
//CHECK-DAG: _slang_atomic_add_u64{{.*}}_slang_coverage{{.*int\(0\)}}
//CHECK-DAG: _slang_atomic_add_u64{{.*}}_slang_coverage{{.*int\(1\)}}
//CHECK-DAG: _slang_atomic_add_u64{{.*}}_slang_coverage{{.*int\(2\)}}

// Exactly three. If the implicit `void` return ever suppressed the split,
// `computeMain` would collapse to one region and slot 2 would disappear.
//CHECK-NOT: _slang_coverage{{.*int\(3\)}}
