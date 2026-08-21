// Pins how coalescing treats calls, which is the most fragile part of
// the region analysis: it decides whether a call keeps a region intact
// or splits it, and getting either direction wrong is silent.
//
// A call to a function that returns normally must NOT split a region.
// If it did, coalescing would be defeated for essentially all code that
// calls helpers, while the metadata still looks healthy — entries would
// still outnumber counters thanks to the call-free statements alone.
//
// A call whose target cannot be resolved statically MUST split. Coverage
// instrumentation runs before `specializeModule`, so an interface method
// dispatched through a witness table has no visible body here and the
// pass has to assume it can abandon the invocation.

//TEST:SIMPLE(filecheck=CHECK):-target cpp -stage compute -entry computeMain -trace-coverage

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<float> outputBuffer;

// Ordinary function: returns normally, so calling it preserves a region.
float doubleIt(float x)
{
    return x * 2.0f;
}

interface IStep { float apply(float x); }
struct AddOne : IStep { float apply(float x) { return x + 1.0f; } }

float viaInterface(IStep step, float x)
{
    // `step.apply` is a witness-table dispatch, so these two statements
    // are one region and the two after the call are another.
    float a = x + 1.0f;
    float b = step.apply(a);
    float c = b + 1.0f;
    return c;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // All five statements are one region despite the `doubleIt` call
    // sitting inside it. The trailing `viaInterface` call cannot split
    // anything, because no marker follows it in this block.
    float v = (float)tid.x;
    float w = doubleIt(v);
    float x = w + 1.0f;
    AddOne step;
    outputBuffer[0] = viaInterface(step, x);
}

// Five regions: `doubleIt`'s body, `viaInterface` either side of the
// witness dispatch, `computeMain`'s single region, and `AddOne::apply`.
//CHECK-DAG: _slang_atomic_add_u64{{.*}}_slang_coverage{{.*int\(0\)}}
//CHECK-DAG: _slang_atomic_add_u64{{.*}}_slang_coverage{{.*int\(1\)}}
//CHECK-DAG: _slang_atomic_add_u64{{.*}}_slang_coverage{{.*int\(2\)}}
//CHECK-DAG: _slang_atomic_add_u64{{.*}}_slang_coverage{{.*int\(3\)}}
//CHECK-DAG: _slang_atomic_add_u64{{.*}}_slang_coverage{{.*int\(4\)}}

// Exactly five. A sixth slot means the `doubleIt` call started splitting
// `computeMain`; slot 4 going missing above means the witness dispatch
// stopped splitting `viaInterface`. The test fails in both directions.
//CHECK-NOT: _slang_coverage{{.*int\(5\)}}
