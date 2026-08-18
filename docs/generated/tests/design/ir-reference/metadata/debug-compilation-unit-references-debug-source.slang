//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#debug-info-family
//META: doc_section_digest=8f873a7e67bec608b74c11364d191e8a9af0122bb32b3a8409d1b57ac5e3d4bd
//META: purpose=DebugCompilationUnit declares the compilation unit with a single operand referencing a DebugSource.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the compilation-unit record's arity and its target. The CHECK pins
// the closing parenthesis after one operand, so a second operand would fail
// the match, and the captured id is then required to be defined by a
// `DebugSource` -- the link the doc describes.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute -g

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK: = DebugCompilationUnit(%[[SRC:[0-9]+]])
// CHECK: %[[SRC]]{{.*}}= DebugSource(
