//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:08+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-struct.md#description
//META: doc_section_digest=0b091f4f0ba7cb2a07817dc55025aaaaadee5292438bfc88fba1ab02ee5d3878
//META: purpose=`static const` data members must have a default initializer; the value is accessible at compile time via the type name.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `static const` members require a default initializer and fold to a literal
// when accessed via the type name. TestStruct.c = 6 is accessed in the entry point;
// CHECK confirms the emitted HLSL contains the folded literal int(6).

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry main -stage compute

struct TestStruct
{
    static const int c = 6;
}

RWStructuredBuffer<int> output;

[numthreads(1,1,1)]
void main()
{
    output[0] = TestStruct.c;
}

// CHECK: int(6)

// GLSL folds the static const to its literal value 6 at the store site.
// GLSL: = 6

// SPIRV emits the static const value as an OpConstant.
// SPV: %int_6 = OpConstant

// Metal folds the static const to int(6).
// METAL: int(6)

// WGSL folds the static const to i32(6).
// WGSL: i32(6)

// CUDA folds the static const to int(6).
// CUDA: int(6)

// CPP folds the static const to int(6).
// CPP: int(6)
