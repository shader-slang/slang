//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -std 2026 -target cpp -entry computeMain -stage compute

// The 2026 companion of `vec4-init-with-3-elements.slang`. Under Slang language
// version 2026 the two confusing 3-component initializers for a 4-component
// vector (see issue #12093) are `[RemovedSince(2026)]`, so each use is now a
// hard error rather than a deprecation warning.
//
// Note: when Slang language version 2026 is retired, bump the `RemovedSince`
// versions on these initializers in `core.meta.slang` and update this test.

static int4 myVec1 = { int2(1, 2), 3 };
//CHECK:             ^ use of removed declaration
//CHECK:             ^ $init has been removed since language version '2026': Three-component initializer (vector<T, 2>, T) for vector<T, 4> has been removed since Slang 2026.

RWStructuredBuffer<int4> output;

void computeMain()
{
    int4 myVec2 = { 1, int2(2, 3) };
//CHECK:          ^ use of removed declaration
//CHECK:          ^ $init has been removed since language version '2026': Three-component initializer (T, vector<T, 2>) for vector<T, 4> has been removed since Slang 2026.

    output[0] = myVec1;
    output[1] = myVec2;
}
