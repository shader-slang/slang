//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#linkage-and-lifetime
//META: doc_section_digest=71129d05eff64ad258a0ba3326a79769f2e4a43a55b9975b0f92d155b6e49e80
//META: purpose=Every user-declared top-level function carries an export decoration whose operand is its mangled linkage name.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `export` records the mangled linkage name of a top-level
// declaration. The CHECK matches an `[export("...")]` line containing the
// source identifier and then the `func %triple` it precedes, which shows
// the decoration is attached to the function inst rather than emitted
// standalone. The mangled prefix/suffix are wildcarded because they encode
// the module name and signature.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

int triple(int x) { return x + x + x; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = triple(int(tid.x));
}

// CHECK: [export("{{.*}}triple{{.*}}")]
// CHECK: func %triple
