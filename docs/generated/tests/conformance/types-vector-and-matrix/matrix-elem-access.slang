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
    printf("t1=%d t2=%d t3=%d %d %d %d t4=%d %d\n",
           t1, t2, t3.x, t3.y, t3.z, t3.w, t4.x, t4.y);
}
