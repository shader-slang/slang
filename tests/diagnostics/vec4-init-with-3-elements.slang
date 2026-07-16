//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -std 2025 -target cpp -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -std legacy -target cpp -entry computeMain -stage compute

// A four-component vector cannot be sensibly initialized from three components.
// The two 3-component initializer forms — `{ vector<T,2>, T }` and
// `{ T, vector<T,2> }` — are confusing (see issue #12093), so they are marked
// `[deprecated]` in the default/legacy language version (this test) and
// `[RemovedSince(2026)]` (see `vec4-init-with-3-elements-2026.slang`).
//
// Because these are only deprecations here, the initializers still compile;
// the point of the test is that each use is diagnosed exactly once, pointing at
// the offending initializer list.

static int4 myVec1 = { int2(1, 2), 3 };
//CHECK:             ^ use of deprecated declaration
//CHECK:             ^ $init has been deprecated: Three-component initializer (vector<T, 2>, T) for vector<T, 4> is deprecated. For details, see https://github.com/shader-slang/slang/issues/12093

RWStructuredBuffer<int4> output;

void computeMain()
{
    int4 myVec2 = { 1, int2(2, 3) };
//CHECK:          ^ use of deprecated declaration
//CHECK:          ^ $init has been deprecated: Three-component initializer (T, vector<T, 2>) for vector<T, 4> is deprecated. For details, see https://github.com/shader-slang/slang/issues/12093

    output[0] = myVec1;
    output[1] = myVec2;
}
