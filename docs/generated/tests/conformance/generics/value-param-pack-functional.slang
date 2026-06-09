//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:43+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/generics.md#description
//META: doc_section_digest=7b3e1c2f4d8a0e9f5b2c6d1e3f7a4b8c0d2e6f9a1b5c3e7f0d4a8b2c6e0f4a
//META: purpose=A generic value parameter pack declared with `let each N : uint` accepts a variadic list of integer values; the pack is expanded element-wise with `expand each`.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc grammar: `generic-value-param-pack-decl` = `let each identifier [: simple-type-spec]`.
// A value parameter pack holds a variable-length sequence of values of the same type.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

void storeOne(inout int total, int val) { total += val; }

int sumValuePack<let each N : uint>() where nonempty(N)
{
    int total = 0;
    expand storeOne(total, each N);
    return total;
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 id : SV_DispatchThreadID)
{
    // Pack bound to three values: 10, 20, 30
    outputBuffer[0] = sumValuePack<10, 20, 30>();
}
// CHECK: 60
