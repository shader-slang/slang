// A `NativeRef<T*>` only needs `T*` declared, so the source emitters forward-declare that pointer
// type whether or not it is on a cycle (shader-slang/slang#13396). A pointer to a type that is
// not a struct needs no forward declaration; a pointer to a struct forward-declares the struct.

//TEST:SIMPLE(filecheck=CHECK): -target cpp -entry computeMain -stage compute -line-directive-mode none

struct N
{
    float v;
    N* next;
}

struct S
{
    NativeRef<int*> r;
    NativeRef<float4*> f;
    NativeRef<int**> pp;
    NativeRef<N*> n;
    int v;
}

RWStructuredBuffer<int> output;
uniform S s;

[numthreads(1, 1, 1)]
void computeMain()
{
    output[0] = s.v;
}

// CHECK: struct [[N:N_[0-9]+]];
// CHECK: int32_t** r_0;
// CHECK: Vector<float, 4>** f_0;
// CHECK: int32_t*** pp_0;
// CHECK: [[N]]** n_0;
