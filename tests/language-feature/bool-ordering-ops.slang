// Verify the results of the ordering comparisons `<`, `<=`, `>` and `>=` on `bool`
// operands, and `<` on `bool2` operands, at run time on every runnable target.
//
// Since #3317, `bool` conforms to `IComparable` through `ILogical`, so the ordering operators
// resolve on Boolean values. Their meaning is fixed by the ordering of `false` and `true`:
// `a < b` is `!a && b`, and `a <= b` is `!a || b`. SPIR-V has no comparison instruction for
// Boolean operands, and GLSL and WGSL do not allow relational operators on Booleans, so a
// scalar comparison must not reach the emitters as a `<` on `bool`; it has to arrive as the
// logical operators that every target supports. The GLSL text target, which cannot be run,
// is compile-checked in tests/cross-compile/glsl-bool-ordering-ops.slang.
//
// The operands are loaded from buffers so that they are only known at run time. Constant
// folding would replace the comparison with its result before code generation and hide what
// each target emits.
//
// Each of the four threads gets one of the four combinations of the inputs `A` and `B`. The
// result of each comparison goes to its own group of four elements in `output`, one element
// per thread.

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

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=output
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

    // Scalar operators. The operands are only known at run time.
    put(0, tid, a < b);
    put(1, tid, a <= b);
    put(2, tid, a > b);
    put(3, tid, a >= b);

    // Generic `min`/`max` route through the same members via the `IComparable`
    // requirement, the other path that used to emit the invalid `Less` on `bool`.
    put(6, tid, min(a, b));
    put(7, tid, max(a, b));

    // Vector operators. `vector<bool, N>` has no ordering operator of its own:
    // `va < vb` resolves by the implicit element conversion `vector<bool,2>` to
    // `vector<int,2>` onto the generated integer `operator<`, a pre-existing
    // path this test pins. The `.x` and `.y` components get different inputs.
    bool2 va = bool2(a, b);
    bool2 vb = bool2(b, a);

    bool2 vless = va < vb;
    put(4, tid, vless.x);
    put(5, tid, vless.y);
}

// scalar a < b
// CHECK: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 0
// scalar a <= b
// CHECK-NEXT: 1
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// scalar a > b
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 0
// CHECK-NEXT: 0
// scalar a >= b
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// vector .x: a < b
// CHECK-NEXT: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 0
// vector .y: b < a
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 0
// CHECK-NEXT: 0
// min(a, b)
// CHECK-NEXT: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// max(a, b)
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
