// Verify the results of the logical bit-operators `&`, `|` and `^` (and `^=`) on `bool` and
// `bool2` operands at run time, on every target.
//
// The operators are not equally supported by the targets. For example, WGSL defines `&` and `|`
// for `bool` and `vecN<bool>` but not `^`, so the WGSL emitter has to turn the exclusive-or into a
// not-equal comparison `!=`. That rewrite changes the operator precedence, so the test also puts
// `==` and `!=` comparisons next to `^`. Every combination of inputs is checked, because
// `a ^ b` and `a != b` agree on all of them only if the emitted code is correct.
//
// Each of the four threads gets one combination of the inputs `A`, `B` and `C`. The result of each
// operation goes to its own group of four elements in `output`, one element per thread.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx11 -profile sm_5_0 -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-mtl -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-wgpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 1 0 1], stride=4):name=A
StructuredBuffer<uint> A;

//TEST_INPUT:ubuffer(data=[0 0 1 1], stride=4):name=B
StructuredBuffer<uint> B;

//TEST_INPUT:ubuffer(data=[0 1 1 0], stride=4):name=C
StructuredBuffer<uint> C;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=output
RWStructuredBuffer<uint> output;

// Writes `value` as 0 or 1 to the element of this thread in the group `group`.
void put(uint group, uint tid, bool value)
{
    if (value)
    {
        output[group * 4 + tid] = 1;
    }
    else
    {
        output[group * 4 + tid] = 0;
    }
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;
    bool a = A[tid] != 0;
    bool b = B[tid] != 0;
    bool c = C[tid] != 0;

    // Scalar operators.
    put(0, tid, a & b);
    put(1, tid, a | b);
    put(2, tid, a ^ b);

    bool compound = a;
    compound ^= b;
    put(3, tid, compound);

    // The operands of `^` are comparisons, which WGSL does not allow to be chained with `!=`.
    put(4, tid, (A[tid] == C[tid]) ^ (B[tid] == C[tid]));
    put(5, tid, (A[tid] != C[tid]) ^ (B[tid] != C[tid]));

    // Vector operators. The `.x` and `.y` components get different inputs.
    bool2 va = bool2(a, b);
    bool2 vb = bool2(c, c);

    bool2 vand = va & vb;
    put(6, tid, vand.x);
    put(7, tid, vand.y);

    bool2 vor = va | vb;
    put(8, tid, vor.x);
    put(9, tid, vor.y);

    bool2 vxor = va ^ vb;
    put(10, tid, vxor.x);
    put(11, tid, vxor.y);

    bool2 vcompound = va;
    vcompound ^= vb;
    put(12, tid, vcompound.x);
    put(13, tid, vcompound.y);
}

// scalar a & b
// CHECK: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// scalar a | b
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// scalar a ^ b
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 0
// scalar a ^= b
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 0
// scalar (a == c) ^ (b == c)
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 0
// scalar (a != c) ^ (b != c)
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 0
// vector .x va & vb
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 0
// CHECK-NEXT: 0
// vector .y va & vb
// CHECK-NEXT: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 0
// vector .x va | vb
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// vector .y va | vb
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// vector .x va ^ vb
// CHECK-NEXT: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// vector .y va ^ vb
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// vector .x va ^= vb
// CHECK-NEXT: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// vector .y va ^= vb
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 0
// CHECK-NEXT: 1
