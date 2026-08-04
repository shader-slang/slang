// A `[noinline]` function with a `__ref` parameter keeps its call boundary.
//
// Each lane checks that the callee survives as a real function and is still called, rather than
// being force-inlined into the entry point for the sake of avoiding a by-reference parameter.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute

RWStructuredBuffer<uint> buf;

[noinline]
void bump(__ref uint v)
{
    v = v + 1;
}

// HLSL emits the callee as a separate function taking an `inout` reference, and calls it.
//HLSL: void bump_0(inout uint
//HLSL: bump_0(

// CUDA lowers the by-reference parameter to a pointer and passes the address of the local.
//CUDA: void bump_0(uint * {{.*}})
//CUDA: bump_0(&

// Metal likewise keeps a pointer-typed parameter across the boundary.
//METAL: void bump_0(uint thread*
//METAL: bump_0(&

// Direct SPIR-V keeps the call and marks the function DontInline.
//SPIRV: OpFunctionCall
//SPIRV: DontInline

// GLSL carries the reference with `spirv_by_reference`. `-emit-spirv-via-glsl` is not checked:
// glslang re-inlines the call, as it does for any `[noinline]` function.
//GLSL: void bump_0(spirv_by_reference uint
//GLSL: bump_0(

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    uint t = buf[0];
    bump(t);
    buf[0] = t;
}
