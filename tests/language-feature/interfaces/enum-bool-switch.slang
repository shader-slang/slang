//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute

// Regression test for shader-slang/slang#12260: a `switch` on an `enum : bool`
// value must compile and select the matching case. The CPU test checks the
// selected case at run time; the SPIR-V and WGSL tests check that the bool
// selector is legalized to an integer switch (these targets require an integer
// selector, so an un-legalized bool switch would fail to compile / emit invalid
// output).

enum Mode : bool
{
    Immediate = false,
    Deferred = true,
}

int classify(Mode m)
{
    switch (m)
    {
    case Mode::Immediate:
        return 10;
    case Mode::Deferred:
        return 20;
    }
    return -1;
}

// The bool selector is converted to an integer (false -> 0, true -> 1) and the
// bool-typed case labels become integer literals, so the switch lowers to an
// integer `OpSwitch`.
// SPIRV: [[SEL:%[0-9]+]] = OpSelect %int {{.*}} %int_1 %int_0
// SPIRV: OpSwitch [[SEL]] %{{[0-9]+}} 0 %{{[0-9]+}} 1 %{{[0-9]+}}

// WGSL likewise requires an integer selector. The bool selector is converted with
// `select(0, 1, cond)` (WGSL's idiom for a bool-to-int value; a bare `i32(cond)` is not
// the form the emitter uses), and the case labels are integers — never a bool selector
// or `case true:`/`case false:`.
// WGSL: switch(select(i32(0), i32(1),
// WGSL: case i32(0)
// WGSL: case i32(1)

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // A constant cast to bool folds by comparing against zero, so any nonzero
    // value is `true`. These share the fold path exercised by the enum switch.
    static_assert((bool)2 == true, "cast of 2 to bool folds to true");
    static_assert((bool)4 == true, "cast of 4 to bool folds to true");
    static_assert((bool)0 == false, "cast of 0 to bool folds to false");

    Mode m = (tid.x % 2 == 0) ? Mode::Immediate : Mode::Deferred;
    outputBuffer[tid.x] = classify(m);
    // CHECK: 10
    // CHECK: 20
    // CHECK: 10
    // CHECK: 20
}
