//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#concepts
//META: doc_section_digest=82fc9ca4bda6256917442becba961ab7af693eb5db7c9a9e9bf691de4244f8b8
//META: purpose=The Attribute category is not part of the default lookup mask, so a value declared with an attribute's spelling neither hides the attribute nor is hidden by it.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the category filter separating attribute decls from
// ordinary values. A file-scope constant is named numthreads, the same
// spelling as the attribute applied to the entry point. Each site asks
// for a different category, so both must resolve independently. CHECK
// asserts the attribute still emits with its own arguments and that the
// buffer write picks up the constant's value 7.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage compute -entry main

static const int numthreads = 7;

RWStructuredBuffer<int> outBuf;

[numthreads(1,1,1)]
void main()
{
    // CHECK: numthreads(1, 1, 1)
    // CHECK: = int(7)
    outBuf[0] = numthreads;
}
