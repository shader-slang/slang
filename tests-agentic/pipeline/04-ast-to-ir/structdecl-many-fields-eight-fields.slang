//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T10:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=7d43267959b0c547516266beea43e6e96622d37270bfab810d9819a48f90812b
//META: purpose=A StructDecl with eight scalar fields lowers to an IRStructType carrying eight `field(...)` children in source order.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: struct-field-count "many" edge. The doc's
// "StructDecl → IRStructType with IRStructField children" claim
// must hold at higher field counts; verifies eight `field(...)`
// rows under the struct header.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

struct Big {
    int a; int b; int c; int d;
    int e; int f; int g; int h;
}

uniform int x;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Big big;
    big.a = x; big.b = x; big.c = x; big.d = x;
    big.e = x; big.f = x; big.g = x; big.h = x;
    buf[0] = big.a + big.b + big.c + big.d
           + big.e + big.f + big.g + big.h;
}

// CHECK-LABEL: struct %Big
// CHECK: field(%a, Int)
// CHECK: field(%b, Int)
// CHECK: field(%c, Int)
// CHECK: field(%d, Int)
// CHECK: field(%e, Int)
// CHECK: field(%f, Int)
// CHECK: field(%g, Int)
// CHECK: field(%h, Int)
