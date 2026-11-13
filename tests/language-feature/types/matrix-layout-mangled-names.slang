// The mangled name of a matrix type includes its layout when the layout is specified, so
// declarations that differ only in a matrix layout get distinct names. A matrix with an
// unspecified layout keeps the `m<R>x<C><T>` spelling. The layout goes between the row count and
// the `x`, so the two `nested` overloads get distinct names, and so do the two `rowMajor`
// overloads, one of whose element type is an `__extern_cpp` struct named like a layout token
// (shader-slang/slang#13383).

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -no-codegen -dump-ir

module matrix_layout_mangled_names;

public float plain(float2x3 m) { return 0; }
public float rowMajor(row_major float2x3 m) { return 1; }
public float columnMajor(column_major float2x3 m) { return 2; }
public float generic<let L : MatrixLayoutMode>(matrix<float, 2, 3, L> m) { return 3; }
public float nested(matrix<matrix<float, 2, 3, MatrixLayoutMode.RowMajor>, 2, 3> m) { return 4; }
public float nested(matrix<matrix<float, 2, 3>, 2, 3, MatrixLayoutMode.RowMajor> m) { return 5; }

__extern_cpp public struct Lk1f
{
    float x;
}
public float rowMajor(matrix<Lk1f, 2, 3> m) { return 6; }

// CHECK-DAG: [export("_S27matrix_layout_mangled_names5plainp1pi_m2x3ff")]
// CHECK-DAG: [export("_S27matrix_layout_mangled_names8rowMajorp1pi_m2Lk1x3ff")]
// CHECK-DAG: [export("_S27matrix_layout_mangled_names11columnMajorp1pi_m2Lk2x3ff")]
// CHECK-DAG: [export("_S27matrix_layout_mangled_names7genericg1v4core16MatrixLayoutModep1pi_m2LK1Lx3ff")]
// CHECK-DAG: [export("_S27matrix_layout_mangled_names6nestedp1pi_m2x3m2Lk1x3ff")]
// CHECK-DAG: [export("_S27matrix_layout_mangled_names6nestedp1pi_m2Lk1x3m2x3ff")]
// CHECK-DAG: [export("_S27matrix_layout_mangled_names8rowMajorp1pi_m2x3Lk1ff")]
