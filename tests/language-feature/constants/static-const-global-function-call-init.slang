//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl

// Regression test for https://github.com/shader-slang/slang/issues/9404
// A `static const` global initialized from a function call is a runtime value
// and must be diagnosed on all targets, not silently called multiple times.

RWStructuredBuffer<int> outputBuffer;

int getX()
{
    return outputBuffer[0];
}

// Direct function call initializer must be diagnosed.
// CHECK: error[E31226]
static const int x = getX();
//CHECK:         ^ static const global initializer must be a compile-time constant
//CHECK:         ^ initializer of static const global 'x' does not evaluate to a compile-time constant

// Arithmetic on a function call result must also be caught.
// CHECK: error[E31226]
static const int y = getX() + 1;
//CHECK:         ^ static const global initializer must be a compile-time constant
//CHECK:         ^ initializer of static const global 'y' does not evaluate to a compile-time constant

// Nested method call (member function on a struct) must also be caught.
struct Helper
{
    int val;
    int get() { return val; }
};

Helper h;

// CHECK: error[E31226]
static const int z = h.get();
//CHECK:         ^ static const global initializer must be a compile-time constant
//CHECK:         ^ initializer of static const global 'z' does not evaluate to a compile-time constant

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[tid.x] = x + y + z;
}
