//DIAGNOSTIC_TEST:SIMPLE(diag=CONCAT_NON_AXIS): -DCONCAT_NON_AXIS
//DIAGNOSTIC_TEST:SIMPLE(diag=CONCAT_RANK): -DCONCAT_RANK
//DIAGNOSTIC_TEST:SIMPLE(diag=CONCAT_AXIS): -DCONCAT_AXIS
//DIAGNOSTIC_TEST:SIMPLE(diag=PERMUTE_LENGTH): -DPERMUTE_LENGTH
//DIAGNOSTIC_TEST:SIMPLE(diag=PERMUTE_DUP): -DPERMUTE_DUP
//DIAGNOSTIC_TEST:SIMPLE(diag=PERMUTE_RANGE): -DPERMUTE_RANGE
//DIAGNOSTIC_TEST:SIMPLE(diag=SWAP_AXIS): -DSWAP_AXIS
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
    MockTile<T, __permuteVals(D, Order)> permute<let each Order : int>()
    {
        MockTile<T, __permuteVals(D, Order)> result;
        return result;
    }

    void touchShape()
    {
        ShapeInfo<D>::touch();
    }
};

MockTile<T, __concatVals(A, B, Axis)> concat<
    let Axis : int,
    T : ICoopElement,
    let each A : int,
    let each B : int>
    (MockTile<T, A> lhs, MockTile<T, B> rhs)
{
    MockTile<T, __concatVals(A, B, Axis)> result;
    return result;
}

MockTile<T, __swapVals(D, I, J)> transpose<
    let I : int,
    let J : int,
    T : ICoopElement,
    let each D : int>
    (MockTile<T, D> tile)
{
    MockTile<T, __swapVals(D, I, J)> result;
    return result;
}

void expect16x64(MockTile<float, 16, 64> tile) {}
void expect32x16(MockTile<float, 32, 16> tile) {}

#ifdef CONCAT_NON_AXIS
void testConcatNonAxis()
{
    expect16x64(concat<1>(MockTile<float, 16, 32>(), MockTile<float, 8, 32>()));
    //CONCAT_NON_AXIS: __concatVals requires non-axis dimensions to match
    //CONCAT_NON_AXIS: __concatVals requires dimensions to match outside axis 1, but dimension 0 differs
}
#endif

#ifdef CONCAT_RANK
void testConcatRank()
{
    expect16x64(concat<1>(MockTile<float, 16, 32>(), MockTile<float, 16, 32, 64>()));
    //CONCAT_RANK: __concatVals requires packs of the same rank
    //CONCAT_RANK: __concatVals requires packs of the same rank, but got ranks 2 and 3
}
#endif

#ifdef CONCAT_AXIS
void testConcatAxis()
{
    expect16x64(concat<2>(MockTile<float, 16, 32>(), MockTile<float, 16, 32>()));
    //CONCAT_AXIS: __concatVals axis is out of range
    //CONCAT_AXIS: axis 2 is out of range for rank 2 in __concatVals
}
#endif

#ifdef PERMUTE_LENGTH
void testPermuteLength()
{
    expect32x16(MockTile<float, 16, 32>().permute<1, 0, 2>());
    //PERMUTE_LENGTH: __permuteVals requires an order pack with matching rank
    //PERMUTE_LENGTH: __permuteVals requires the order pack to have length 3 matching the input rank 2
}
#endif

#ifdef PERMUTE_DUP
void testPermuteDuplicate()
{
    expect32x16(MockTile<float, 16, 32>().permute<0, 0>());
    //PERMUTE_DUP: __permuteVals order contains a duplicate index
    //PERMUTE_DUP: __permuteVals order index 0 is duplicated at positions 0 and 1
}
#endif

#ifdef PERMUTE_RANGE
void testPermuteRange()
{
    expect32x16(MockTile<float, 16, 32>().permute<0, 2>());
    //PERMUTE_RANGE: __permuteVals index is out of range
    //PERMUTE_RANGE: __permuteVals order index 2 at position 1 is out of range for rank 2
}
#endif

#ifdef SWAP_AXIS
void testSwapAxis()
{
    expect32x16(transpose<0, 2>(MockTile<float, 16, 32>()));
    //SWAP_AXIS: __swapVals axis is out of range
    //SWAP_AXIS: axis 2 is out of range for rank 2 in __swapVals
}
#endif
