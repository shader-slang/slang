// ref-accessor-targets.slang
//
// A property `ref` accessor lowers to a helper that returns a pointer to the
// referenced storage. On a target with first-class pointers (Metal) that helper
// survives and returns a typed pointer; on targets without them (HLSL, GLSL,
// WGSL) — and on SPIR-V, whose Logical addressing model forbids returning a
// `Function`-storage pointer — the helper is inlined so the write folds into the
// call site and stores through the field directly.
// https://github.com/shader-slang/slang/issues/12487, #9636

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
        [mutating] ref { return _v; }
    }
}

RWStructuredBuffer<int> outputBuffer;

// The pointer-less targets (HLSL, GLSL, WGSL) inline the helper and store into
// the field directly — no pointer-returning function survives. Metal and CUDA
// have pointer types and keep the helper returning a typed pointer. SPIR-V
// inlines and the module validates.

// HLSL-NOT: Cell_value_ref
// HLSL: _v_0 = int(13)

// GLSL-NOT: Cell_value_ref
// GLSL: _v_0 = 13

// WGSL-NOT: Cell_value_ref
// WGSL: _v_0 = i32(13)

// METAL: int thread*{{.*}}Cell_value_ref

// CUDA: int * Cell_value_ref

// SPIRV: result code = 0

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    Cell c;
    c._v = 0;
    c.value = 13;
    outputBuffer[0] = c.value;
}
