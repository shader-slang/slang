//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#struct-and-class-containers
//META: doc_section_digest=35e316a9e45af6f156a356bfecea45802765e6938f01700b134ee12ff89ff959
//META: purpose=Boundary: a single-field struct surfaces exactly one `field` row under the parent.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// a single-field struct surfaces exactly one `field` row under the
// parent. The minimum nontrivial member count (1) surfaces exactly one
// `field(%key, <T>)` row.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o - -stage compute -entry main

struct Single { int x; }

uniform Single s;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = s.x;
}

// IR-LABEL: struct %Single
// IR: field(%{{.*}}, Int)
