//DIAGNOSTIC_TEST:SIMPLE(diag=LATE_CONCAT_NON_AXIS): -target spirv -entry computeMain -stage compute -DLATE_CONCAT_NON_AXIS
//DIAGNOSTIC_TEST:SIMPLE(diag=LATE_CONCAT_RANK): -target spirv -entry computeMain -stage compute -DLATE_CONCAT_RANK
//DIAGNOSTIC_TEST:SIMPLE(diag=LATE_CONCAT_AXIS): -target spirv -entry computeMain -stage compute -DLATE_CONCAT_AXIS
//DIAGNOSTIC_TEST:SIMPLE(diag=LATE_PERMUTE_LENGTH): -target spirv -entry computeMain -stage compute -DLATE_PERMUTE_LENGTH
//DIAGNOSTIC_TEST:SIMPLE(diag=LATE_PERMUTE_DUP): -target spirv -entry computeMain -stage compute -DLATE_PERMUTE_DUP
//DIAGNOSTIC_TEST:SIMPLE(diag=LATE_PERMUTE_RANGE): -target spirv -entry computeMain -stage compute -DLATE_PERMUTE_RANGE
//DIAGNOSTIC_TEST:SIMPLE(diag=LATE_SWAP_AXIS): -target spirv -entry computeMain -stage compute -DLATE_SWAP_AXIS
//DIAGNOSTIC_TEST:SIMPLE(diag=LATE_REDUCE_AXIS): -target spirv -entry computeMain -stage compute -DLATE_REDUCE_AXIS
//DIAGNOSTIC_TEST:SIMPLE(diag=LATE_SWAP_SAME_AXIS): -target spirv -entry computeMain -stage compute -DLATE_SWAP_SAME_AXIS
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

void userConcat<
    let Axis : int,
    T : ICoopElement,
    let each A : int,
    let each B : int>
    (MockTile<T, A> lhs, MockTile<T, B> rhs)
{
    concat<Axis, T, A, B>(lhs, rhs).touchShape();
}

struct UserPermute<let each Order : int>
{
    static void apply<T : ICoopElement, let each D : int>(
        MockTile<T, D> tile)
    {
        tile.permute<Order>().touchShape();
    }
};

void userTranspose<
    let I : int,
    let J : int,
    T : ICoopElement,
    let each D : int>
    (MockTile<T, D> tile)
{
    transpose<I, J, T, D>(tile).touchShape();
}

struct UserReduce<let Axis : int>
{
    static void apply<T : ICoopElement, let each D : int>(
        MockTile<T, D> tile)
    {
        tile.reduce<Axis>().touchShape();
    }
};

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
#ifdef LATE_CONCAT_NON_AXIS
    userConcat<1>(MockTile<float, 16, 32>(), MockTile<float, 8, 32>());
    //LATE_CONCAT_NON_AXIS: __shapeConcat requires non-axis dimensions to match
    //LATE_CONCAT_NON_AXIS: __shapeConcat requires dimensions to match outside axis 1, but dimension 0 differs
#endif

#ifdef LATE_CONCAT_RANK
    userConcat<1>(MockTile<float, 16, 32>(), MockTile<float, 16, 32, 64>());
    //LATE_CONCAT_RANK: __shapeConcat requires packs of the same rank
    //LATE_CONCAT_RANK: __shapeConcat requires packs of the same rank, but got ranks 2 and 3
#endif

#ifdef LATE_CONCAT_AXIS
    userConcat<2>(MockTile<float, 16, 32>(), MockTile<float, 16, 32>());
    //LATE_CONCAT_AXIS: __shapeConcat axis is out of range
    //LATE_CONCAT_AXIS: axis 2 is out of range for rank 2 in __shapeConcat
#endif

#ifdef LATE_PERMUTE_LENGTH
    UserPermute<1, 0, 2>::apply(MockTile<float, 16, 32>());
    //LATE_PERMUTE_LENGTH: __shapePermute requires an order pack with matching rank
    //LATE_PERMUTE_LENGTH: __shapePermute requires the order pack to have length 3 matching the input rank 2
#endif

#ifdef LATE_PERMUTE_DUP
    UserPermute<0, 0>::apply(MockTile<float, 16, 32>());
    //LATE_PERMUTE_DUP: __shapePermute order contains a duplicate index
    //LATE_PERMUTE_DUP: __shapePermute order index 0 is duplicated at positions 0 and 1
#endif

#ifdef LATE_PERMUTE_RANGE
    UserPermute<0, 2>::apply(MockTile<float, 16, 32>());
    //LATE_PERMUTE_RANGE: __shapePermute index is out of range
    //LATE_PERMUTE_RANGE: __shapePermute order index 2 at position 1 is out of range for rank 2
#endif

#ifdef LATE_SWAP_AXIS
    userTranspose<0, 2>(MockTile<float, 16, 32>());
    //LATE_SWAP_AXIS: __shapeSwap axis is out of range
    //LATE_SWAP_AXIS: axis 2 is out of range for rank 2 in __shapeSwap
#endif

#ifdef LATE_SWAP_SAME_AXIS
    userTranspose<2, 2>(MockTile<float, 16, 32>());
    //LATE_SWAP_SAME_AXIS: __shapeSwap axis is out of range
    //LATE_SWAP_SAME_AXIS: axis 2 is out of range for rank 2 in __shapeSwap
#endif

#ifdef LATE_REDUCE_AXIS
    UserReduce<2>::apply(MockTile<float, 16, 32>());
    //LATE_REDUCE_AXIS: __shapeReduce axis is out of range
    //LATE_REDUCE_AXIS: axis 2 is out of range for rank 2 in __shapeReduce
#endif
}
