//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T15:30:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/types-vector-and-matrix.md#row-and-element-access-
//META: doc_section_digest=b0ab4edd7e624713af836700667d410c7afa9b2f65ed30df6db9b64ee04184ba
//META: purpose=Matrix element access: `m[r][c]` returns the value at row r, column c. `m[r]` returns a row vector. `m[r].yx` swizzles the row.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Direct port of the doc's example matrix into INTERPRET form.

//TEST:INTERPRET(filecheck=CHECK):
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry main -stage compute

// Matrix subscript access `m[r][c]`, row-vector access `m[r]`, and `.yx`
// swizzle on a row. HLSL preserves the matrix type `int3x4` and uses
// `m_0[int(r)][int(c)]` / `m_0[int(r)].w` notation. GLSL/SPIRV fully
// constant-fold (not checked). CUDA/CPP use `m_0.rows[r].component`.
// Metal/WGSL: printf unsupported in compute; untested for emission.

void main()
{
    matrix<int, 3, 4> m = {
        1,  2,  3,  4,
        5,  6,  7,  8,
        9, 10, 11, 12
    };
    int t1 = m[1][2]; // row 1, col 2  = 7
    int t2 = m[1].w;  // row 1, col 3  = 8
    int4 t3 = m[2];   // row 2        = { 9, 10, 11, 12 }
    int2 t4 = m[0].yx;// { m[0].y=2, m[0].x=1 } = { 2, 1 }
    //CHECK: t1=7 t2=8 t3=9 10 11 12 t4=2 1
    // On HLSL, matrix type is int3x4; element access uses m_0[r][c] and .w notation.
    //HLSL-DAG: int3x4
    //HLSL-DAG: m_0[int(1)][int(2)]
    //HLSL-DAG: m_0[int(0)].yx
    // On CUDA, rows array with component swizzle.
    //CUDA-DAG: Matrix<int, 3, 4>
    //CUDA-DAG: m_0.rows[int(1)].z
    //CUDA-DAG: m_0.rows[int(0)].y, m_0.rows[int(0)].x
    // On CPP, same Matrix template with rows.
    //CPP-DAG: Matrix<int32_t, 3, 4>
    //CPP-DAG: m_0.rows[int(1)].z
    //CPP-DAG: m_0.rows[int(0)].y, m_0.rows[int(0)].x
    printf("t1=%d t2=%d t3=%d %d %d %d t4=%d %d\n",
           t1, t2, t3.x, t3.y, t3.z, t3.w, t4.x, t4.y);
}
