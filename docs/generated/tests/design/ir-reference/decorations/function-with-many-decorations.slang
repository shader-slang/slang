//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00Z
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#inlining-and-optimization
//META: doc_section_digest=99f72cda245d99360168c24acf24a82fd577e139a911dfa073addcf5493b365a
//META: purpose=Stress - a single function carrying many compatible inlining-and-optimization decorations stacked together produces every corresponding decoration in the dump, with no interference between them.
//META: intent=stress
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress - the doc lists many independent decorations in the
// inlining-and-optimization category. Stack four user-side
// attributes on one helper ([ForceInline], [__NoSideEffect],
// [__readNone], [__unsafe_force_inline_early]) plus the
// front-end-attached nameHint and export. All four should appear
// in the dump for that one function.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[ForceInline]
[__NoSideEffect]
[__readNone]
[__unsafe_force_inline_early]
int superHelper(int x) { return x + 1; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = superHelper(int(tid.x));
}

// CHECK-DAG: [ForceInline]
// CHECK-DAG: [noSideEffect]
// CHECK-DAG: [readNone]
// CHECK-DAG: [unsafeForceInlineEarly]
// CHECK-DAG: [nameHint("superHelper")]
