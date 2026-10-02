// The mangled name of a matrix type includes its layout when the layout is not the default, so
// declarations that differ only in a matrix layout get distinct names. A plain matrix keeps the
// `m<R>x<C><T>` spelling, and the layout is placed before the element type, so the two `nested`
// overloads below are told apart (shader-slang/slang#13383).

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -no-codegen -dump-ir

module matrix_layout_mangled_names;

public float plain(float2x3 m) { return 0; }
public float rowMajor(row_major float2x3 m) { return 1; }
public float columnMajor(column_major float2x3 m) { return 2; }
public float generic<let L : MatrixLayoutMode>(matrix<float, 2, 3, L> m) { return 3; }
public float nested(matrix<matrix<float, 2, 3, MatrixLayoutMode.RowMajor>, 2, 3> m) { return 4; }
public float nested(matrix<matrix<float, 2, 3>, 2, 3, MatrixLayoutMode.RowMajor> m) { return 5; }

// CHECK-DAG: [export("_S27matrix_layout_mangled_names5plainp1pi_m2x3ff")]
// CHECK-DAG: [export("_S27matrix_layout_mangled_names8rowMajorp1pi_m2x3Lk1ff")]
// CHECK-DAG: [export("_S27matrix_layout_mangled_names11columnMajorp1pi_m2x3Lk2ff")]
// CHECK-DAG: [export("_S27matrix_layout_mangled_names7genericg1v4core16MatrixLayoutModep1pi_m2x3LK1Lff")]
// CHECK-DAG: [export("_S27matrix_layout_mangled_names6nestedp1pi_m2x3m2x3Lk1ff")]
// CHECK-DAG: [export("_S27matrix_layout_mangled_names6nestedp1pi_m2x3Lk1m2x3ff")]
