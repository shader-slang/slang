//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T17:00:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/expressions-member-access.md#matrix-swizzles
//META: doc_section_digest=ab827430dedad798221ccb6a8d05027ec37ed381ec7b70905cec740e9ed7e864
//META: purpose=Matrix zero-based `_mij` swizzle: `m._m12` reads row 1, column 2 (zero-indexed).
//META: intent=boundary
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    matrix<int, 3, 3> m = {
        10, 11, 12,
        20, 21, 22,
        30, 31, 32,
    };
    //CHECK: m00=10 m12=22 m21=31
    printf("m00=%d m12=%d m21=%d\n",
           m._m00, m._m12, m._m21);
}
