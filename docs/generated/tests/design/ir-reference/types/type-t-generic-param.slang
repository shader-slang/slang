//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#spir-v-literals-and-kinds
//META: doc_section_digest=ef66016fdd706005d0d8fa43c10be7a90ee7d5b5099b8b6fab751a3d3d27cc5e
//META: purpose=A generic type-parameter is typed type_t in the IR dump.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A generic type-parameter is typed type_t in the IR dump. A `T
// identity<T>(T x)` declaration shows `param %T : type_t` inside the
// surrounding `generic` instruction.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

T identity<T>(T x) { return x; }

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = identity<int>(a);
}

// IR: generic
// IR: param %T {{.*}} type_t
