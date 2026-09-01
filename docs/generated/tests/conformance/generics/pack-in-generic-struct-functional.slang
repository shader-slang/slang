//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/language-reference/generics.md#type-parameter-packs-type-param-packs
//META: doc_section_digest=1d7c4d9f36d4d30dc9d8e89006174f0691c6f26a9618f3cb1da30611dc1fe7df
//META: purpose=A generic struct can store a type parameter pack by expanding it into a `Tuple` field, and the tuple elements are accessible after specialization.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that an expand-expression is usable in a type position inside a
// generic struct declaration: `Holder<each T>` declares a field of type
// `Tuple<expand each T>`, so the pack expands into the tuple's element types.
// Specializing at `<int, int>` gives a two-element tuple; the entry point
// assigns `makeTuple(3, 4)` and reads both elements back. CHECK asserts 7,
// proving the pack expanded into the field's type and both elements survived
// specialization.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

struct Holder<each T>
{
    Tuple<expand each T> items;
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Holder<int, int> h;
    h.items = makeTuple(3, 4);
    outputBuffer[0] = h.items._0 + h.items._1;
}
// CHECK: 7
