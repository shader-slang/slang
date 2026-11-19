// The source emitters break a `Ptr(N)` → `N` → `Ptr(N)` cycle at the pointer
// (shader-slang/slang#13396). GLSL forward-declares the pointer's `buffer_reference` block, because
// it cannot forward-declare a struct; HLSL, Metal, CUDA and C++ forward-declare the struct. The
// entry points start the walk from opposite ends of the cycle: `pointerFirst` reaches `Ptr(N)`
// through the `cbuffer`, while `structFirst` and `structOnly` reach `N` first. `structOnly` never
// uses the pointer, so its GLSL `buffer_reference` block is defined only because `N` was.

//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry pointerFirst -stage compute
//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry structFirst -stage compute
//TEST:SIMPLE(filecheck=VIA_GLSL): -target spirv -emit-spirv-via-glsl -entry pointerFirst -stage compute
//TEST:SIMPLE(filecheck=VIA_GLSL): -target spirv -emit-spirv-via-glsl -entry structFirst -stage compute
//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry structOnly -stage compute
//TEST:SIMPLE(filecheck=VIA_GLSL): -target spirv -emit-spirv-via-glsl -entry structOnly -stage compute
//TEST:SIMPLE(filecheck=CLIKE): -target hlsl -entry pointerFirst -stage compute
//TEST:SIMPLE(filecheck=CLIKE): -target hlsl -entry structFirst -stage compute
//TEST:SIMPLE(filecheck=CLIKE): -target metal -entry pointerFirst -stage compute
//TEST:SIMPLE(filecheck=CLIKE): -target metal -entry structFirst -stage compute
//TEST:SIMPLE(filecheck=CLIKE): -target cuda -entry pointerFirst -stage compute
//TEST:SIMPLE(filecheck=CLIKE): -target cuda -entry structFirst -stage compute
//TEST:SIMPLE(filecheck=CLIKE): -target cpp -entry pointerFirst -stage compute
//TEST:SIMPLE(filecheck=CLIKE): -target cpp -entry structFirst -stage compute
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry pointerFirst -stage compute
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry structFirst -stage compute

struct N
{
    float v;
    N* next;
};

cbuffer PointerFirst
{
    N* p;
}

cbuffer StructFirst
{
    N n;
}

RWStructuredBuffer<float> output;

[numthreads(1, 1, 1)]
void pointerFirst()
{
    output[0] = p->next->v;
}

[numthreads(1, 1, 1)]
void structFirst()
{
    output[0] = n.next->v;
}

[numthreads(1, 1, 1)]
void structOnly()
{
    output[0] = n.v;
}

// GLSL: layout(buffer_reference) buffer [[PTR:BufferPointer_N_[0-9_]+]];
// GLSL: struct [[N:N_[0-9]+]]
// GLSL: [[PTR]] next_0;
// GLSL: layout(buffer_reference, std430, buffer_reference_align = 8) buffer [[PTR]]
// GLSL-NEXT: {
// GLSL-NEXT: [[N]] _data;

// VIA_GLSL: OpEntryPoint

// CLIKE: struct [[N:N_[0-9]+]];
// CLIKE: struct [[N]]
// CLIKE-NEXT: {
// CLIKE: next_0;

// WGSL only checks that emission no longer aborts: the `ptr<, N_0>` member it emits is not valid
// WGSL (shader-slang/slang#13399).
// WGSL-NOT: struct N_0;
// WGSL: struct N_0{{$}}
// WGSL: next_0 : ptr<
