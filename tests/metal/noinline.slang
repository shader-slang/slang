//TEST:SIMPLE(filecheck=METAL): -target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -entry computeMain -stage compute -O1

// A Slang `[noinline]` request must reach the Metal compiler. Without the MSL attribute,
// Metal is free to inline the helper even though the Slang IR retains its noInline decoration.

// METAL: __attribute__((noinline))
// METAL-NEXT: float helper_{{[0-9]+}}(float value_{{[0-9]+}})

// The metallib arm uses an optimized downstream compile, where the helper would be inlined without
// the attribute, and verifies that it survives. It auto-skips without the Metal compiler.
// METALLIB: helper

[noinline]
float helper(float value)
{
    return value * value + 1.0;
}

RWStructuredBuffer<float> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    output[0] = helper(2.0);
}
