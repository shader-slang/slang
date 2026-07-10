//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T00:00:00+00:00
//META: source_commit=1ee81dae21143fb69ccb9999f0fd013b4db2d922
//META: doc_ref=docs/generated/design/pipeline/overview.md#end-to-end-flow
//META: doc_section_digest=050f2a1afed02c0a6b611d78eee6fa52f2d4cdf8d79a760ffe0b1f192d46e37b
//META: purpose=A non-trivial source successfully traverses the full pipeline to produce HLSL text with the entry-point body intact.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Fans the full-pipeline end-to-end claim out to the Metal back-end:
// the same macro/struct/callee/if-statement source must reach a Metal
// artefact through the whole pipeline. CHECK pins the Metal compute
// marker (kernel), the lowered struct, the thread-address-space callee
// pointer parameter, and the branch so each stage's contribution is
// visible in the emitted MSL.

//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute

#define LIMIT 4

struct Counter
{
    int value;
};

int bump(Counter c)
{
    return c.value + 1;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    Counter c;
    c.value = LIMIT;
    int next = bump(c);
    if (next > 0)
    {
        return;
    }
    return;
}

//METAL: int bump_0(const Counter_0 thread* c_{{[0-9]+}})
//METAL: void main_0()
//METAL: if(_S{{[0-9]+}} > int(0))
