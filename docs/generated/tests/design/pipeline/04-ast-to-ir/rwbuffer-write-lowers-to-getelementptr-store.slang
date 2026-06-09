//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=7d43267959b0c547516266beea43e6e96622d37270bfab810d9819a48f90812b
//META: purpose=A subscript-assignment on an `RWStructuredBuffer<T>` lowers to `rwstructuredBufferGetElementPtr` followed by `store`.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists `IRFieldAddress`/`IRFieldExtract` for `MemberExpr`,
// but the analogous lowering for `buf[i]` on an `RWStructuredBuffer`
// is `rwstructuredBufferGetElementPtr` (the resource analogue of
// `get_field_addr`). The doc's general claim — that an lvalue
// subscript on a buffer becomes an address-of + store — is
// observable via the two-instruction sequence in the IR dump.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = a;
}

// CHECK-LABEL: func %main
// CHECK: rwstructuredBufferGetElementPtr(%buf, 0 : Int)
// CHECK: store(%{{.*}}, %{{.*}})
