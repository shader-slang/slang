//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#error-code-namespace
//META: doc_section_digest=c07669430fa1f58c4db53ae38c50d1523f8399ab4e7aa698054222ac63e9d41b
//META: purpose=Boundary: `-warnings-disable` accepts a comma-separated list and applies the override to every identifier in it.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the list form of the request at its smallest interesting size, two
// entries, and mixes the two accepted identifier kinds. One shader triggers both
// 41024 and 20103; the request names the first by integer id and the second by
// name, and CHECK-NOT asserts both warnings disappear together.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute -warnings-disable 41024,keyword-used-as-name

RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    int x = (1, 2);
    int enum = x;
    buf[0] = enum;
}

// CHECK-NOT: E41024
// CHECK-NOT: E20103
// CHECK-NOT: comma operator used in expression
// CHECK-NOT: is a type keyword
// CHECK: void main
