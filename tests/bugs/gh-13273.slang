// Regression test for #13273: an array index read before a store to the index
// variable must not be re-evaluated after that store in textual targets.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type -emit-spirv-via-glsl
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -compute -shaderobj -output-using-type -use-dxil
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type

// We also check the generated source, because the CPU and CUDA runtime paths above only
// reproduce the `push` shape.
//TEST:SIMPLE(filecheck=SRC): -target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SRC): -target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SRC): -target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SRC): -target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SRC): -target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SRC): -target cpp -entry computeMain -stage compute

//TEST_INPUT:ubuffer(data=[16 83 0 0 0 0 0 0], stride=4):out,name=buf
RWStructuredBuffer<uint> buf;

struct Stack
{
    uint a[8];
    int top;
}

// SRC-LABEL: pop_0(
// SRC: [[POP_IDX:_S[0-9]+]] {{.*}}= {{.*}}top_0;
// SRC: top_0 = {{.*}}top_0 -
// SRC: a_0{{\[}}[[POP_IDX]]{{\]}} /
uint pop(inout Stack s)
{
    uint f = s.a[s.top];
    s.top -= 1;
    return f / 16u;
}

// SRC-LABEL: popTwoUses_0(
// SRC: [[TWO_IDX:_S[0-9]+]] {{.*}}= {{.*}}top_0;
// SRC: top_0 = {{.*}}top_0 -
// SRC: handle_0{{.*}} = {{.*}}a_0{{\[}}[[TWO_IDX]]{{\]}} /
void popTwoUses(inout Stack s, out uint handle, out uint depth)
{
    uint f = s.a[s.top];
    s.top -= 1;
    handle = f / 16u;
    depth = f & 15u;
}

uint bump(inout Stack s)
{
    s.top += 1;
    return 7;
}

// SRC-LABEL: push_0(
// SRC: [[PUSH_IDX:_S[0-9]+]] {{.*}}= {{.*}}top_0;
// SRC: bump_0(
// SRC: a_0{{\[}}[[PUSH_IDX]]{{\]}} =
void push(inout Stack s)
{
    s.a[s.top] = bump(s);
}

// SRC-LABEL: popBelow_0(
// SRC: [[BELOW_IDX:_S[0-9]+]] {{.*}}= {{.*}}top_0 -
// SRC: top_0 = {{.*}}top_0 -
// SRC: a_0{{\[}}[[BELOW_IDX]]{{\]}};
uint popBelow(inout Stack s)
{
    uint f = s.a[s.top - 1];
    s.top -= 1;
    return f;
}

Stack makeStack(int top)
{
    Stack s;
    s.a[0] = buf[0];
    s.a[1] = buf[1];
    s.top = top;
    return s;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    Stack s0 = makeStack(1);
    buf[2] = pop(s0);

    Stack s1 = makeStack(1);
    uint handle, depth;
    popTwoUses(s1, handle, depth);
    buf[3] = handle;
    buf[4] = depth;

    Stack s2 = makeStack(0);
    push(s2);
    buf[5] = s2.a[0];
    buf[6] = s2.a[1];

    Stack s3 = makeStack(2);
    buf[7] = popBelow(s3);
}

// CHECK: 16
// CHECK-NEXT: 83
// CHECK-NEXT: 5
// CHECK-NEXT: 5
// CHECK-NEXT: 3
// CHECK-NEXT: 7
// CHECK-NEXT: 83
// CHECK-NEXT: 83
