// Regression test for #13273: an array index read before a store to the index
// variable must not be re-evaluated after that store in textual targets.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type -emit-spirv-via-glsl
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -compute -shaderobj -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type

// We also check the generated source, because a wrong fold of `pop`, `popTwoUses`,
// `popBelow`, `popField` or `loopPop` does not change the results of the CPU and CUDA runtime
// tests above, so only the source checks would catch it there.
//TEST:SIMPLE(filecheck=SRC): -target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SRC): -target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SRC): -target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SRC): -target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SRC): -target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SRC): -target cpp -entry computeMain -stage compute

//TEST_INPUT:ubuffer(data=[16 83 0 0 0 0 0 0 0 0], stride=4):out,name=buf
RWStructuredBuffer<uint> buf;

struct Stack
{
    uint a[8];
    int top;
}

// Each function reads or writes an element indexed by `s.top` and also changes `s.top`. A
// wrong fold emits the index expression after that change, so it uses the wrong element.

// The element load is deferred past the store to `s.top`, and the index reaches it through
// an always-folded `getElementPtr`.
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

// The same shape with two uses of the deferred load.
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

// No load is deferred: the element address is emitted in the store, after the call that
// changes `s.top`. Slang evaluates the destination `s.a[s.top]` before the right-hand side.
// SRC-LABEL: push_0(
// SRC: [[PUSH_IDX:_S[0-9]+]] {{.*}}= {{.*}}top_0;
// SRC: bump_0(
// SRC: a_0{{\[}}[[PUSH_IDX]]{{\]}} =
void push(inout Stack s)
{
    s.a[s.top] = bump(s);
}

// The index goes through a `sub`, which is folded only after passing the same check.
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

struct Entry
{
    uint node;
    uint pad;
}

struct EntryStack
{
    Entry e[4];
    int top;
}

// The index reaches the load through two always-folded instructions, a `getElementPtr` and a
// `FieldAddress`.
// SRC-LABEL: popField_0(
// SRC: [[FIELD_IDX:_S[0-9]+]] {{.*}}= {{.*}}top_{{[0-9]+}};
// SRC: top_{{[0-9]+}} = {{.*}}top_{{[0-9]+}} -
// SRC: e_0{{\[}}[[FIELD_IDX]]{{\]}}{{.*}}node_0
uint popField(inout EntryStack s)
{
    uint f = s.e[s.top].node;
    s.top -= 1;
    return f;
}

// The load is used by the loop exit condition as well as after the loop.
// SRC-LABEL: loopPop_0(
// SRC: [[LOOP_IDX:_S[0-9]+]] {{.*}}= {{.*}}top_0;
// SRC: top_0 = {{.*}}top_0 -
// SRC: a_0{{\[}}[[LOOP_IDX]]{{\]}}{{.*}} < {{.*}}20
uint loopPop(inout Stack s)
{
    uint f;
    for (;;)
    {
        f = s.a[s.top];
        s.top -= 1;
        if (f < 20u)
            break;
    }
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

    EntryStack es;
    es.e[0].node = buf[0];
    es.e[1].node = buf[1];
    es.top = 1;
    buf[8] = popField(es);

    Stack s4 = makeStack(1);
    buf[9] = loopPop(s4);
}

// buf[2]: pop gives 83 / 16; a wrong fold gives 16 / 16 = 1.
// buf[3], buf[4]: popTwoUses gives 83 / 16 and 83 & 15.
// buf[5], buf[6]: push writes 7 to a[0]; a wrong fold writes it to a[1].
// buf[7]: popBelow gives a[1]; a wrong fold gives a[0].
// buf[8]: popField gives e[1].node; a wrong fold gives e[0].node.
// buf[9]: loopPop gives a[0] on the second iteration; a wrong fold exits the first with a[1].
// CHECK: 16
// CHECK-NEXT: 83
// CHECK-NEXT: 5
// CHECK-NEXT: 5
// CHECK-NEXT: 3
// CHECK-NEXT: 7
// CHECK-NEXT: 83
// CHECK-NEXT: 83
// CHECK-NEXT: 83
// CHECK-NEXT: 16
