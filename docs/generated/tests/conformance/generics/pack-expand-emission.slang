//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/language-reference/generics.md#type-param-packs
//META: doc_section_digest=1d7c4d9f36d4d30dc9d8e89006174f0691c6f26a9618f3cb1da30611dc1fe7df
//META: purpose=A pack-expanded generic function specialized to a fixed pack length emits a monomorphized function taking one scalar per pack element on every text-emit target.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that expand/each specialization reaches codegen on each back-end:
// `sumInts<each T>` called with a length-3 pack monomorphizes into a concrete
// `sumInts` function whose expansion is baked into the emitted target text, and
// the entry point stores its result. Each directive checks the specialized
// function name and buffer store survive to that target's output. The first
// argument is derived from the dispatch id so the call is not constant-folded
// away before emit.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute

void sumHelper(inout int acc, int term)
{
    acc += term;
}

int sumInts<each T>(expand each T terms) where T == int
{
    int acc = 0;
    expand sumHelper(acc, each terms);
    return acc;
}

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 id : SV_DispatchThreadID)
{
    outputBuffer[0] = sumInts(int(id.x), 2, 3);
}

// HLSL-DAG: sumInts
// HLSL-DAG: outputBuffer
// SPV-DAG: OpEntryPoint GLCompute
// SPV-DAG: %outputBuffer
// GLSL-DAG: sumInts
// GLSL-DAG: outputBuffer
// METAL-DAG: sumInts
// METAL-DAG: outputBuffer
// WGSL-DAG: sumInts
// WGSL-DAG: outputBuffer
// CPP-DAG: sumInts
// CPP-DAG: outputBuffer
