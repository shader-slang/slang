//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00Z
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#loop-and-branch-hints
//META: doc_section_digest=d7cf9915ec0da5de567edb118ec898dd57bb3e7b5414db216254906018825537
//META: purpose=Negative - [ForceUnroll] on a function declaration is rejected with E31002 "invalid attribute placement"; the attribute is loop-statement-only despite the doc listing its AST origin generically as "[ForceUnroll] attribute".
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Negative - the BUNDLE doc gaps already flag that ForceUnroll is
// loop-statement-only. This test pins the diagnostic that fires
// when the attribute is misplaced on a function declaration.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -stage compute -entry main
uniform RWStructuredBuffer<int> buf;

[ForceUnroll]
/*CHECK:
 ^^^^^^^^^^^ attribute 'ForceUnroll' is not valid here
*/
// CHECK: E31002
int helper(int x) { return x + 1; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = helper(int(tid.x));
}
