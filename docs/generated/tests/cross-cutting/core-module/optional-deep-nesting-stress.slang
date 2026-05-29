//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00Z
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#what-the-core-module-provides
//META: doc_section_digest=cf6e88293f416086490f900fb6322b3e953eeecf6ea19c2143f0800c759fa6c5
//META: purpose=Stress: the core-module `Optional<T>` type composes under nesting (Optional<Optional<Optional<int>>>).
//META: intent=stress
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress axis: generic-instantiation recursion depth >= 3 — the
// core-module `Optional<T>` type nested three deep.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Optional<Optional<Optional<int>>> deep = Optional<Optional<Optional<int>>>(
        Optional<Optional<int>>(
            Optional<int>(int(tid.x))));
    if (deep.hasValue && deep.value.hasValue && deep.value.value.hasValue)
    {
        outBuf[tid.x] = deep.value.value.value;
    }
    else
    {
        outBuf[tid.x] = -1;
    }
}

// HLSL: void main
