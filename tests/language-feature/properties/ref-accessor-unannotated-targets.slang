// ref-accessor-unannotated-targets.slang
//
// The same cross-target check as ref-accessor-targets.slang, for an UNANNOTATED
// `ref` accessor (no `[mutating]`). An unannotated `ref` is implicitly mutating,
// so `this` is passed by reference and the emitted code matches the `[mutating]`
// spelling: pointer-less targets (HLSL, GLSL, WGSL) inline the helper and store
// into the field directly, while Metal and CUDA keep a typed-pointer helper that
// receives `this` by pointer. This is the `ref`-accessor shape from #9636.
// https://github.com/shader-slang/slang/issues/9636, #12487

//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL): -target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -entry computeMain -stage compute -emit-spirv-directly

struct Cell
{
    int _v;
    property int value
    {
        get { return _v; }
        ref { return _v; } // no `[mutating]` — implicitly mutating
    }
}

RWStructuredBuffer<int> outputBuffer;

// HLSL-NOT: Cell_value_ref
// HLSL: _v_0 = int(13)

// GLSL-NOT: Cell_value_ref
// GLSL: _v_0 = 13

// WGSL-NOT: Cell_value_ref
// WGSL: _v_0 = i32(13)

// The helper both returns a typed pointer and receives `this` by pointer/
// reference (into the caller's `Cell`), which is what makes the write land
// rather than mutating a callee-local copy.
// METAL: int thread*{{.*}}Cell_value_ref{{.*}}Cell_0 thread*

// CUDA: int *{{.*}}Cell_value_ref{{.*}}Cell_0 *

// SPIRV: result code = 0

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    Cell c;
    c._v = 0;
    c.value = 13;
    outputBuffer[0] = c.value;
}
