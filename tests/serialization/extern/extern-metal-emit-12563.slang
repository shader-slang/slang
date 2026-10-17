//TEST:COMPILE: -no-codegen tests/serialization/extern/module-a.slang -o tests/serialization/extern/module-a-12563.slang-lib
//TEST:COMPILE: -no-codegen tests/serialization/extern/module-b.slang -o tests/serialization/extern/module-b-12563.slang-lib
//TEST:SIMPLE(filecheck=METAL): -target metal -stage compute -entry computeMain -r tests/serialization/extern/module-a-12563.slang-lib -r tests/serialization/extern/module-b-12563.slang-lib

// Regression test for shader-slang/slang#12563 (found while reviewing the fix
// for shader-slang/slang#12498).
//
// This is the compile-only Metal counterpart of extern-test.slang. `computeMain`
// calls `doSomething` (module-b), which calls `foo` (module-a); both take a
// `Thing` by value, which lowers to a pointer parameter that the address-space
// specialization pass specializes to the caller's `thread` address space. So
// both the original `doSomething` and the original `foo` become dead. Removing
// them with a single, order-dependent pass could visit the original `foo` while
// it is still used by the not-yet-removed original `doSomething`, skip it, then
// remove `doSomething` — leaving `foo` behind as a dead function whose parameter
// keeps a Generic address space that Metal emit cannot lower ("Unknown
// addressspace encountered"). extern-test.slang only exercises this on the macOS
// CI jobs (it runs on a device); compiling to Metal source needs no device, so
// this runs the same path on every platform.

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

// Declare the type exists.
extern struct Thing {};
// A mechanism to make a Thing without knowing the specific fields.
extern Thing makeThing(int a, int b);

extern int doSomething(Thing a, Thing b);

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    int index = int(dispatchThreadID.x);

    Thing a = makeThing(index + 1, index + 2);
    Thing b = makeThing(-index, index * 2);

    // METAL: {{\[\[}}kernel{{\]\]}} void computeMain
    outputBuffer[index] = doSomething(a, b);
}
