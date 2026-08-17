//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#debug-info-family
//META: doc_section_digest=8f873a7e67bec608b74c11364d191e8a9af0122bb32b3a8409d1b57ac5e3d4bd
//META: purpose=DebugFunction declares a function for the debugger with its name, line, column, source file and function type, and the owning function links to it by decoration.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the five operands of the function record and the decoration that
// points at it. Debug level 1 is used so the source-text operand of the
// module's source record stays empty, which keeps the dump free of an embedded
// copy of this file. The CHECK captures the source id, requires the function
// record to name it in operand 3 between the line/column literals and the
// function type, and finally requires the record id to appear inside a
// debug-function decoration -- the link from a function back to its record.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute -g1

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK: %[[SRC:[0-9]+]]{{.*}}= DebugSource(
// CHECK: %[[DF:[0-9]+]]{{.*}}= DebugFunction("main", {{[0-9]+}} : UInt, {{[0-9]+}} : UInt, %[[SRC]], Func(
// CHECK: [DebugFunction(%[[DF]])]
