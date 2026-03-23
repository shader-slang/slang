//TEST:INTERPRET(filecheck=CHECK):
#lang slang 2026

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

MockTile<T, __shapeConcat(D, (__first(D), 1), 1)> appendUnitColumn<
    T : ICoopElement,
    let each D : int>
    (MockTile<T, D> tile) where nonempty(D)
{
    MockTile<T, __shapeConcat(D, (__first(D), 1), 1)> result;
    return result;
}

MockTile<T, __shapeConcat((1, __last(D)), D, 0)> prependUnitRow<
    T : ICoopElement,
    let each D : int>
    (MockTile<T, D> tile) where nonempty(D)
{
    MockTile<T, __shapeConcat((1, __last(D)), D, 0)> result;
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
void expect16x1(MockTile<float, 16, 1> tile) {}
void expect16x33(MockTile<float, 16, 33> tile) {}
void expect17x32(MockTile<float, 17, 32> tile) {}
void expect16x32x8(MockTile<float, 16, 32, 8> tile) {}
void expect16x1x8(MockTile<float, 16, 1, 8> tile) {}
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

    let reduced2D = concatTile.reduce<1>();
    expect16x1(reduced2D);
    printShape(reduced2D);
    // CHECK: 16
    // CHECK: 1

    let widened2D = appendUnitColumn(lhs);
    expect16x33(widened2D);
    printShape(widened2D);
    // CHECK: 16
    // CHECK: 33

    let padded2D = prependUnitRow(lhs);
    expect17x32(padded2D);
    printShape(padded2D);
    // CHECK: 17
    // CHECK: 32

    let volume = MockTile<float, 8, 16, 32>();
    let permuted3D = volume.permute<1, 2, 0>();
    expect16x32x8(permuted3D);
    printShape(permuted3D);
    // CHECK: 16
    // CHECK: 32
    // CHECK: 8

    let reduced3D = permuted3D.reduce<1>();
    expect16x1x8(reduced3D);
    printShape(reduced3D);
    // CHECK: 16
    // CHECK: 1
    // CHECK: 8

    let transposed3D = transpose<0, 2>(permuted3D);
    expect8x32x16(transposed3D);
    printShape(transposed3D);
    // CHECK: 8
    // CHECK: 32
    // CHECK: 16

    expect16x64(transpose<0, 1>(concat<1>(lhs, rhs).permute<1, 0>()));
    expect16x1(reduced2D);
    expect16x1x8(reduced3D);
}
