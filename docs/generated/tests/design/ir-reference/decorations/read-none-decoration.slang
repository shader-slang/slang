//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00Z
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#inlining-and-optimization
//META: doc_section_digest=99f72cda245d99360168c24acf24a82fd577e139a911dfa073addcf5493b365a
//META: purpose=Boundary - a function carrying [__readNone] surfaces a bare readNone decoration distinct from noSideEffect, ForceInline, and the other inlining-and-optimization opcodes.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary - the doc lists readNone under inlining-and-optimization
// alongside ForceInline and noSideEffect, with AST origin
// "[readnone] attribute". The user-side spelling that surfaces
// the decoration is `[__readNone]` (the documented `[readnone]`
// is an unknown attribute in the checker). Pin the resulting
// bare decoration.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[__readNone]
int pureFn(int x) { return x * 2; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = pureFn(int(tid.x));
}

// CHECK: [readNone]
// CHECK: [nameHint("pureFn")]
// CHECK: func %pureFn
