// Regression test for the scoping of an array index in textual targets. When the element
// address after a branch is the one computed inside the branch, the index is declared inside
// the branch, so the code after the branch must reach it through a variable declared outside.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type -emit-spirv-via-glsl
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -compute -shaderobj -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type

// glslang rejects GLSL that names a variable outside its scope, so this line checks the GLSL
// output without a GPU.
//TEST:SIMPLE(filecheck=SPV): -target spirv-asm -emit-spirv-via-glsl -entry computeMain -stage compute
// SPV: OpEntryPoint

//TEST:SIMPLE(filecheck=SRC): -target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SRC): -target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SRC): -target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SRC): -target wgsl -entry computeMain -stage compute

// buf[0..3] fill the arrays, and buf[4] = 1 is the branch condition, index and stack top,
// read from the buffer so that the branches are not folded away.
//TEST_INPUT:ubuffer(data=[16 83 5 9 1 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=buf
RWStructuredBuffer<uint> buf;

struct S
{
    uint a[4];
    uint b[4];
    int top;
}

// In the optimized IR, the index `t * 2` has a single use, in the element address inside the
// `if`.
// SRC-LABEL: branchIndex_0(
// SRC: [[VAR:_S[0-9]+]]{{.*}} = {{(int\(|i32\()?}}0{{\)?}};
// SRC: if
// SRC: [[IDX:_S[0-9]+]] {{.*}}= {{.*}}t_0 *
// SRC: [[VAR]] = [[IDX]];
// SRC: a_0{{\[}}[[IDX]]{{\]}}
// SRC: a_0{{\[}}[[VAR]]{{\]}} =
uint branchIndex(inout S s, bool c, int t)
{
    uint x;
    if (c)
        x = s.a[t * 2];
    else
        return 0;
    s.a[t * 2] = x + 7;
    return x;
}

// Both reads share a branch-local index; the later store must not reference it out of scope.
uint branchIndexTwoUses(inout S s, bool c, int t)
{
    uint x;
    if (c)
        x = s.a[t * 2] + s.b[t * 2];
    else
        return 0;
    s.a[t * 2] = x + 7;
    return x;
}

// The index is read inside the `if` and must not be re-read after the store to `s.top`.
// SRC-LABEL: branchIndexAfterStore_0(
// SRC: [[SVAR:_S[0-9]+]]{{.*}} = {{(int\(|i32\()?}}0{{\)?}};
// SRC: if
// SRC: [[SIDX:_S[0-9]+]] {{.*}}= {{.*}}top_0;
// SRC: [[SVAR]] = [[SIDX]];
// SRC: top_0 = {{.*}}top_0 -
// SRC: a_0{{\[}}[[SVAR]]{{\]}} =
uint branchIndexAfterStore(inout S s, bool c)
{
    uint x;
    int i;
    if (c)
    {
        i = s.top;
        x = s.a[i];
    }
    else
        return 0;
    s.top -= 1;
    s.a[i] = x + 7;
    return x;
}

// A case-local index reused by the store after the switch must remain in scope.
uint switchIndex(inout S s, int c, int t)
{
    uint x;
    switch (c)
    {
    case 1:
        x = s.a[t * 2];
        break;
    default:
        return 0;
    }
    s.a[t * 2] = x + 7;
    return x;
}

groupshared uint gs[4];

// The same shape with a groupshared array instead of an `inout` parameter.
uint groupsharedIndex(bool c, int t)
{
    uint x;
    if (c)
        x = gs[t * 2];
    else
        return 0;
    gs[t * 2] = x + 7;
    return x;
}

struct Entry
{
    uint node;
    uint pad;
}

struct EntryArray
{
    Entry e[4];
}

// Two always-folded instructions, a `FieldAddress` of a `getElementPtr`, are copied to the
// store after the `if`, and the copy of the inner one is made for the copy of the outer one.
// SRC-LABEL: branchField_0(
// SRC: [[FVAR:_S[0-9]+]]{{.*}} = {{(int\(|i32\()?}}0{{\)?}};
// SRC: if
// SRC: [[FIDX:_S[0-9]+]] {{.*}}= {{.*}}t_{{[0-9]+}} *
// SRC: [[FVAR]] = [[FIDX]];
// SRC: e_0{{\[}}[[FIDX]]{{\]}}{{.*}}node_0
// SRC: e_0{{\[}}[[FVAR]]{{\]}}{{.*}}node_0 =
uint branchField(inout EntryArray s, bool c, int t)
{
    uint x;
    if (c)
        x = s.e[t * 2].node;
    else
        return 0;
    s.e[t * 2].node = x + 7;
    return x;
}

S makeS()
{
    S s;
    for (int i = 0; i < 4; i++)
    {
        s.a[i] = buf[i];
        s.b[i] = buf[i] * 2;
    }
    s.top = int(buf[4]);
    return s;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    int one = int(buf[4]);

    S s0 = makeS();
    buf[5] = branchIndex(s0, one != 0, one);
    buf[6] = s0.a[2];

    S s1 = makeS();
    buf[7] = branchIndexTwoUses(s1, one != 0, one);
    buf[8] = s1.a[2];

    S s2 = makeS();
    buf[9] = branchIndexAfterStore(s2, one != 0);
    buf[10] = s2.a[0];
    buf[11] = s2.a[1];

    S s3 = makeS();
    buf[12] = switchIndex(s3, one, one);
    buf[13] = s3.a[2];

    for (int i = 0; i < 4; i++)
        gs[i] = buf[i];
    buf[14] = groupsharedIndex(one != 0, one);
    buf[15] = gs[2];

    EntryArray ea;
    for (int i = 0; i < 4; i++)
    {
        ea.e[i].node = buf[i];
        ea.e[i].pad = 0;
    }
    buf[16] = branchField(ea, one != 0, one);
    buf[17] = ea.e[2].node;
}

// Each function returns the element it reads and adds 7 to the element it stores to; a
// regression either fails to compile or stores to the wrong element.
// buf[5], buf[6]: branchIndex reads a[2] = 5 and stores 12 there.
// buf[7], buf[8]: branchIndexTwoUses reads a[2] + b[2] = 15 and stores 22 to a[2].
// buf[9] to buf[11]: branchIndexAfterStore reads a[1] = 83 and stores 90 to a[1], not a[0].
// buf[12], buf[13]: switchIndex, as branchIndex.
// buf[14], buf[15]: groupsharedIndex, as branchIndex.
// buf[16], buf[17]: branchField reads e[2].node = 5 and stores 12 there.

// CHECK: 16
// CHECK-NEXT: 83
// CHECK-NEXT: 5
// CHECK-NEXT: 9
// CHECK-NEXT: 1
// CHECK-NEXT: 5
// CHECK-NEXT: 12
// CHECK-NEXT: 15
// CHECK-NEXT: 22
// CHECK-NEXT: 83
// CHECK-NEXT: 16
// CHECK-NEXT: 90
// CHECK-NEXT: 5
// CHECK-NEXT: 12
// CHECK-NEXT: 5
// CHECK-NEXT: 12
// CHECK-NEXT: 5
// CHECK-NEXT: 12
