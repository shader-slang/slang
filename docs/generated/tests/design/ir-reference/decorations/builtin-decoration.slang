//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#naming-and-provenance
//META: doc_section_digest=6831ab52a37fd97537e855fa7a9e66d014490da47978c08179ccda7326652d30
//META: purpose=Core-module-builtin interfaces such as IBufferDataLayout carry a BuiltinDecoration in the IR dump.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `BuiltinDecoration` is the nullary marker distinguishing a
// compiler-builtin inst from user code. `RWStructuredBuffer` always links
// the core-module `IBufferDataLayout` interface, so the CHECK sequence
// pins the decoration, the interface's `nameHint`, and the declaration
// itself in the order the dump prints them.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o - -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK: [BuiltinDecoration]
// CHECK: [nameHint("IBufferDataLayout")]
// CHECK: let  %IBufferDataLayout
