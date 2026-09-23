//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute

// Regression test for shader-slang/slang#12260: a `switch` on an `enum : bool`
// value must compile and select the matching case. The CPU test checks the
// selected case at run time; the SPIR-V and WGSL tests check that the bool
// selector is legalized to an integer switch (these targets require an integer
// selector, so an un-legalized bool switch would fail to compile / emit invalid
// output).
//
// Regression test for shader-slang/slang#12298: on the C-family targets that do
// NOT legalize the switch to an integer selector (HLSL, C++, CUDA, Metal), the
// bool-typed case labels must be emitted as `false`/`true`, not as `int8_t(0)`/
// `int8_t(1)`.

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

// The C-family targets keep the bool selector and must spell the case labels as the
// canonical bool literals `false`/`true` (`Immediate = false` first, `Deferred = true`
// second); the `-NOT` between the labels pins that no integer `int8_t` spelling is emitted.
// HLSL: case false:
// HLSL-NOT: int8_t
// HLSL: case true:

// CPP: case false:
// CPP-NOT: int8_t
// CPP: case true:

// CUDA: case false:
// CUDA-NOT: int8_t
// CUDA: case true:

// METAL: case false:
// METAL-NOT: int8_t
// METAL: case true:

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
