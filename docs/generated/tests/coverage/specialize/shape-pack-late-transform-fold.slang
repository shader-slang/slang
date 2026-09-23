//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Symbolic shape-pack transforms (__shapeConcat / __shapePermute / __shapeSwap / __shapeReduce) that survive front-end folding are folded by the IR specialization pass, producing the concatenated / permuted / swapped / reduced dimension lists.
//META: intent=characterization
//META: covers=source/slang/slang-ir-specialize.cpp
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterizes the *success* half of `maybeSpecializeShapePackTransformInst`
// in `slang-ir-specialize.cpp`. The hand-written suite already pins the error
// half of that routine (`tests/language-feature/generics/
// diagnose-mock-tile-shape-pack-ops-specialize.slang` drives eight late
// diagnostics through it), and it pins the *front-end* folding of the same
// operators (`mock-tile-shape-pack-ops.slang`), but nothing drives a shape
// transform that both survives front-end folding and then succeeds in the IR
// pass. That is the code that actually rewrites the shape pack.
//
// The trick that keeps the transform symbolic past the front end is the extra
// `user*` indirection layer, copied from the hand-written late-diagnostic
// test: `userConcat<Axis, T, A, B>` forwards its own generic parameters into
// `concat<Axis, T, A, B>`, so at check time the operands of `__shapeConcat`
// are still generic parameters and the checker cannot evaluate it. The
// operands only become concrete when `userConcat<1>(...)` is specialized in
// the IR, at which point `maybeSpecializeShapePackTransformInst` sees a
// `ShapeConcat` over two concrete `MakeValuePack`s and folds it.
//
// `touchShape()` prints the resulting dimension list one value per line, so
// the CHECK block below is the observed shape after folding:
//   concat axis 1 of (16,32) and (16,8)  -> (16,40)   [axis dims added]
//   concat axis 0 of (4,8)  and (4,8)    -> (8,8)
//   permute (16,32) by order (1,0)       -> (32,16)
//   permute (4,8,16) by order (2,0,1)    -> (16,4,8)
//   swap axes 0,1 of (16,32)             -> (32,16)
//   swap axes 1,1 of (16,32)             -> (16,32)   [identity fast path]
//   swap axes 0,2 of (4,8,16)            -> (16,8,4)  [middle axis untouched]
//   reduce axis 1 of (16,32)             -> (16,1)
//   reduce axis 0 of (4,8,16)            -> (1,8,16)
//
// The rank-3 cases matter for coverage as well as for meaning: they are what
// exercises the "this axis is neither of the two being swapped, copy it
// through" branch of the swap fold, which a rank-2 swap can never reach.

//TEST:INTERPRET(filecheck=CHECK):
#lang slang 2026

struct ShapeInfo<let each D : int>
{
    static void touch()
    {
        expand printf("%d\n", each D);
    }
}

struct MockTile<T : ICoopElement, let each D : int>
{
    MockTile<T, __shapePermute(D, Order)> permute<let each Order : int>()
    {
        MockTile<T, __shapePermute(D, Order)> result;
        return result;
    }

    MockTile<T, __shapeReduce(D, Axis)> reduce<let Axis : int>()
    {
        MockTile<T, __shapeReduce(D, Axis)> result;
        return result;
    }

    void touchShape()
    {
        ShapeInfo<D>::touch();
    }
};

MockTile<T, __shapeConcat(A, B, Axis)> concat<
    let Axis : int,
    T : ICoopElement,
    let each A : int,
    let each B : int>
    (MockTile<T, A> lhs, MockTile<T, B> rhs)
{
    MockTile<T, __shapeConcat(A, B, Axis)> result;
    return result;
}

MockTile<T, __shapeSwap(D, I, J)> transpose<
    let I : int,
    let J : int,
    T : ICoopElement,
    let each D : int>
    (MockTile<T, D> tile)
{
    MockTile<T, __shapeSwap(D, I, J)> result;
    return result;
}

// The `user*` wrappers below are the indirection that keeps each shape
// transform unevaluated until IR specialization: every operand is a generic
// parameter of the wrapper, so the checker has nothing constant to fold.

void userConcat<let Axis : int, T : ICoopElement, let each A : int, let each B : int>
    (MockTile<T, A> lhs, MockTile<T, B> rhs)
{
    concat<Axis, T, A, B>(lhs, rhs).touchShape();
}

struct UserPermute<let each Order : int>
{
    static void apply<T : ICoopElement, let each D : int>(MockTile<T, D> tile)
    {
        tile.permute<Order>().touchShape();
    }
};

void userTranspose<let I : int, let J : int, T : ICoopElement, let each D : int>
    (MockTile<T, D> tile)
{
    transpose<I, J, T, D>(tile).touchShape();
}

struct UserReduce<let Axis : int>
{
    static void apply<T : ICoopElement, let each D : int>(MockTile<T, D> tile)
    {
        tile.reduce<Axis>().touchShape();
    }
};

void main()
{
    printf("concat1\n");
    userConcat<1>(MockTile<float, 16, 32>(), MockTile<float, 16, 8>());
    // CHECK-LABEL: concat1
    // CHECK-NEXT: 16
    // CHECK-NEXT: 40

    printf("concat0\n");
    userConcat<0>(MockTile<float, 4, 8>(), MockTile<float, 4, 8>());
    // CHECK-LABEL: concat0
    // CHECK-NEXT: 8
    // CHECK-NEXT: 8

    printf("permute\n");
    UserPermute<1, 0>::apply(MockTile<float, 16, 32>());
    // CHECK-LABEL: permute
    // CHECK-NEXT: 32
    // CHECK-NEXT: 16

    printf("permute3\n");
    UserPermute<2, 0, 1>::apply(MockTile<float, 4, 8, 16>());
    // CHECK-LABEL: permute3
    // CHECK-NEXT: 16
    // CHECK-NEXT: 4
    // CHECK-NEXT: 8

    printf("transpose\n");
    userTranspose<0, 1>(MockTile<float, 16, 32>());
    // CHECK-LABEL: transpose
    // CHECK-NEXT: 32
    // CHECK-NEXT: 16

    printf("transpose-same\n");
    userTranspose<1, 1>(MockTile<float, 16, 32>());
    // CHECK-LABEL: transpose-same
    // CHECK-NEXT: 16
    // CHECK-NEXT: 32

    printf("transpose3\n");
    userTranspose<0, 2>(MockTile<float, 4, 8, 16>());
    // CHECK-LABEL: transpose3
    // CHECK-NEXT: 16
    // CHECK-NEXT: 8
    // CHECK-NEXT: 4

    printf("reduce\n");
    UserReduce<1>::apply(MockTile<float, 16, 32>());
    // CHECK-LABEL: reduce
    // CHECK-NEXT: 16
    // CHECK-NEXT: 1

    printf("reduce3\n");
    UserReduce<0>::apply(MockTile<float, 4, 8, 16>());
    // CHECK-LABEL: reduce3
    // CHECK-NEXT: 1
    // CHECK-NEXT: 8
    // CHECK-NEXT: 16
}
