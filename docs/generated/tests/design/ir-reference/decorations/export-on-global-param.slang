//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#linkage-and-lifetime
//META: doc_section_digest=71129d05eff64ad258a0ba3326a79769f2e4a43a55b9975b0f92d155b6e49e80
//META: purpose=A user-declared uniform global parameter carries an export decoration with the variable-linkage mangled name (prefix _SV).
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `export` is attached to global parameters, not only to
// functions, and that the mangled name it carries is namespaced by symbol
// kind. The CHECK pins the `_SV` variable prefix together with the
// parameter's `nameHint` and the `let %buf` declaration line, so the
// decoration is tied to the global it names.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK: [nameHint("buf")]
// CHECK: [export("_SV{{.*}}buf{{.*}}")]
// CHECK: let  %buf
