//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:48Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/llm-generated/ir-reference/decorations.md#linkage-and-lifetime
//META: doc_section_digest=51a07b0aba2f9f7d1b6cdf9777d9f2f83e7fbd3758ac6f0a6e034ab8f5d4454b
//META: purpose=Every user-declared top-level function carries an export decoration whose operand is its mangled linkage name.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc records `export` ("Marks an inst as exported under a
// mangled name") attached to user `export` declarations and
// synthesized for top-level functions in the lowering.
// The mangled-name string varies with module name, but it always
// begins with `_S`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

int triple(int x) { return x + x + x; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = triple(int(tid.x));
}

// CHECK: [export("{{.*}}triple{{.*}}")]
// CHECK: func %triple
