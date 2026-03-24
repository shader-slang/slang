//DIAGNOSTIC_TEST:SIMPLE(diag=CONCAT_NON_AXIS): -DCONCAT_NON_AXIS
//DIAGNOSTIC_TEST:SIMPLE(diag=CONCAT_RANK): -DCONCAT_RANK
//DIAGNOSTIC_TEST:SIMPLE(diag=CONCAT_AXIS): -DCONCAT_AXIS
//DIAGNOSTIC_TEST:SIMPLE(diag=CONCAT_NO_VALID_AXIS): -DCONCAT_NO_VALID_AXIS
//DIAGNOSTIC_TEST:SIMPLE(diag=PERMUTE_LENGTH): -DPERMUTE_LENGTH
//DIAGNOSTIC_TEST:SIMPLE(diag=PERMUTE_DUP): -DPERMUTE_DUP
//DIAGNOSTIC_TEST:SIMPLE(diag=PERMUTE_DUP_SYMBOLIC): -DPERMUTE_DUP_SYMBOLIC
//DIAGNOSTIC_TEST:SIMPLE(diag=PERMUTE_RANGE): -DPERMUTE_RANGE
//DIAGNOSTIC_TEST:SIMPLE(diag=SWAP_AXIS): -DSWAP_AXIS
//DIAGNOSTIC_TEST:SIMPLE(diag=SWAP_SAME_AXIS): -DSWAP_SAME_AXIS
//DIAGNOSTIC_TEST:SIMPLE(diag=REDUCE_AXIS): -DREDUCE_AXIS
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

void expect16x64(MockTile<float, 16, 64> tile) {}
void expect32x16(MockTile<float, 32, 16> tile) {}

#ifdef CONCAT_NON_AXIS
void testConcatNonAxis()
{
    expect16x64(concat<1>(MockTile<float, 16, 32>(), MockTile<float, 8, 32>()));
    //CONCAT_NON_AXIS: __shapeConcat requires non-axis dimensions to match
    //CONCAT_NON_AXIS: __shapeConcat requires dimensions to match outside axis 1, but dimension 0 differs
}
#endif

#ifdef CONCAT_RANK
void testConcatRank()
{
    expect16x64(concat<1>(MockTile<float, 16, 32>(), MockTile<float, 16, 32, 64>()));
    //CONCAT_RANK: __shapeConcat requires packs of the same rank
    //CONCAT_RANK: __shapeConcat requires packs of the same rank, but got ranks 2 and 3
}
#endif

#ifdef CONCAT_AXIS
void testConcatAxis()
{
    expect16x64(concat<2>(MockTile<float, 16, 32>(), MockTile<float, 16, 32>()));
    //CONCAT_AXIS: __shapeConcat axis is out of range
    //CONCAT_AXIS: axis 2 is out of range for rank 2 in __shapeConcat
}
#endif

#ifdef CONCAT_NO_VALID_AXIS
void testConcatNoValidAxis<let Axis : int>()
{
    expect16x64(concat<Axis>(MockTile<float, 16, 32>(), MockTile<float, 8, 64>()));
    //CONCAT_NO_VALID_AXIS: __shapeConcat has no valid axis
    //CONCAT_NO_VALID_AXIS: no axis in rank 2 can make __shapeConcat valid for these shapes
}
#endif

#ifdef PERMUTE_LENGTH
void testPermuteLength()
{
    expect32x16(MockTile<float, 16, 32>().permute<1, 0, 2>());
    //PERMUTE_LENGTH: __shapePermute requires an order pack with matching rank
    //PERMUTE_LENGTH: __shapePermute requires the order pack to have length 3 matching the input rank 2
}
#endif

#ifdef PERMUTE_DUP
void testPermuteDuplicate()
{
    expect32x16(MockTile<float, 16, 32>().permute<0, 0>());
    //PERMUTE_DUP: __shapePermute order contains a duplicate index
    //PERMUTE_DUP: __shapePermute order index 0 is duplicated at positions 0 and 1
}
#endif

#ifdef PERMUTE_DUP_SYMBOLIC
void testPermuteDuplicateSymbolic<let I : int>()
{
    expect32x16(MockTile<float, 16, 32>().permute<I, I>());
    //PERMUTE_DUP_SYMBOLIC: __shapePermute order contains a duplicate index
    //PERMUTE_DUP_SYMBOLIC: __shapePermute order positions 0 and 1 are provably equal, so the order contains a duplicate index
}
#endif

#ifdef PERMUTE_RANGE
void testPermuteRange()
{
    expect32x16(MockTile<float, 16, 32>().permute<0, 2>());
    //PERMUTE_RANGE: __shapePermute index is out of range
    //PERMUTE_RANGE: __shapePermute order index 2 at position 1 is out of range for rank 2
}
#endif

#ifdef SWAP_AXIS
void testSwapAxis()
{
    expect32x16(transpose<0, 2>(MockTile<float, 16, 32>()));
    //SWAP_AXIS: __shapeSwap axis is out of range
    //SWAP_AXIS: axis 2 is out of range for rank 2 in __shapeSwap
}
#endif

#ifdef SWAP_SAME_AXIS
void testSwapSameAxis()
{
    expect32x16(transpose<2, 2>(MockTile<float, 16, 32>()));
    //SWAP_SAME_AXIS: __shapeSwap axis is out of range
    //SWAP_SAME_AXIS: axis 2 is out of range for rank 2 in __shapeSwap
}
#endif

#ifdef REDUCE_AXIS
void testReduceAxis()
{
    expect32x16(MockTile<float, 16, 32>().reduce<2>());
    //REDUCE_AXIS: __shapeReduce axis is out of range
    //REDUCE_AXIS: axis 2 is out of range for rank 2 in __shapeReduce
}
#endif
