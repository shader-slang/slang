//TEST:INTERPRET(filecheck=CHECK):
#lang slang 2026

struct MockTile<T : ICoopElement, let each D : int>
{
    MockTile<T, __permuteVals(D, Order)> permute<let each Order : int>()
    {
        MockTile<T, __permuteVals(D, Order)> result;
        return result;
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

struct ShapeInfo<let each D : int>
{
    static void print()
    {
        expand printf("%d\n", each D);
    }
}

void printShape<T : ICoopElement, let each D : int>(MockTile<T, D> tile)
{
    ShapeInfo<D>::print();
}

void expect16x64(MockTile<float, 16, 64> tile) {}
void expect64x16(MockTile<float, 64, 16> tile) {}
void expect16x32x8(MockTile<float, 16, 32, 8> tile) {}
void expect8x32x16(MockTile<float, 8, 32, 16> tile) {}

void main()
{
    let lhs = MockTile<float, 16, 32>();
    let rhs = MockTile<float, 16, 32>();
    let concatTile = concat<1>(lhs, rhs);
    expect16x64(concatTile);
    printShape(concatTile);
    // CHECK: 16
    // CHECK: 64

    let transposed2D = transpose<0, 1>(concatTile);
    expect64x16(transposed2D);
    printShape(transposed2D);
    // CHECK: 64
    // CHECK: 16

    let volume = MockTile<float, 8, 16, 32>();
    let permuted3D = volume.permute<1, 2, 0>();
    expect16x32x8(permuted3D);
    printShape(permuted3D);
    // CHECK: 16
    // CHECK: 32
    // CHECK: 8

    let transposed3D = transpose<0, 2>(permuted3D);
    expect8x32x16(transposed3D);
    printShape(transposed3D);
    // CHECK: 8
    // CHECK: 32
    // CHECK: 16

    expect16x64(transpose<0, 1>(concat<1>(lhs, rhs).permute<1, 0>()));
}
