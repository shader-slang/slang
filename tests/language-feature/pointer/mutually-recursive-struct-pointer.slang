// Structs that refer to each other through pointers, reached from a struct (shader-slang/slang#13396).
// The walk opens `A`, then `B`, and forward-declares `Ptr(A)` until `A` is complete, so HLSL, CUDA
// and C++ get `struct A;` ahead of `B`. `C` and `D` form the same kind of cycle through a pointer to
// an array, `Ptr(C[2])`, whose element struct is the one forward-declared.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -compute -output-using-type -shaderobj
//TEST:SIMPLE(filecheck=VIA_GLSL): -target spirv -emit-spirv-via-glsl -entry followAB -stage compute
//TEST:SIMPLE(filecheck=CLIKE): -target hlsl -entry follow -stage compute
//TEST:SIMPLE(filecheck=CLIKE): -target cpp -entry follow -stage compute

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=output
RWStructuredBuffer<float> output;

struct A
{
    float x;
    B* b;
};

struct B
{
    float y;
    A* a;
};

struct C
{
    float x;
    D d;
};

struct D
{
    float y;
    Ptr<C[2]> pair;
};

cbuffer Params
{
    A root;
    C cs[2];
}

[numthreads(1, 1, 1)]
void computeMain()
{
    // The test binds no data to `Params`, so its pointers read as null and its floats as zero.
    output[0] = (root.b == nullptr) ? 1.0 : 0.0;
    output[1] = (cs[1].d.pair == nullptr) ? 1.0 : 0.0;
    output[2] = root.x + cs[0].x;
}

// `follow` and `followAB` dereference the pointers, so they are only compiled, not run.
// `followAB` leaves out `pair`, because indexing through a pointer needs
// `GL_EXT_buffer_reference2`, which the GLSL emitter does not request.
[numthreads(1, 1, 1)]
void follow()
{
    output[0] = root.b->a->x + (*cs[0].d.pair)[1].d.y;
}

[numthreads(1, 1, 1)]
void followAB()
{
    output[0] = root.b->a->x;
}

// CHECK: type: float
// CHECK-NEXT: 1.0
// CHECK-NEXT: 1.0
// CHECK-NEXT: 0.0

// VIA_GLSL: OpEntryPoint

// CLIKE: struct [[A:A_[0-9]+]];
// CLIKE: struct B_{{[0-9]+}}
// CLIKE: struct [[A]]{{$}}
