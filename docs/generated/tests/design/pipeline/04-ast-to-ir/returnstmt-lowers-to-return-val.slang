//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=7d43267959b0c547516266beea43e6e96622d37270bfab810d9819a48f90812b
//META: purpose=A ReturnStmt lowers to an `IRReturn` terminator (spelled `return_val(...)` in the IR dump).
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A user-defined function with `return v;` shows `return_val(%v)` in
// the IR. A `void` function (here `main`) shows
// `return_val(void_constant)`. This is the doc's "ReturnStmt → IRReturn"
// claim. The dump opcode name is `return_val`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

int returnsInt(int x) { return x + 1; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = returnsInt(a);
}

// CHECK-LABEL: func %returnsInt
// CHECK: return_val(%{{.*}})
// CHECK-LABEL: func %main
// CHECK: return_val(void_constant)
