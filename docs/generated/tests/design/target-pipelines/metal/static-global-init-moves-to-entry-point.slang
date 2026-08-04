//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:50:00Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#undoparametercopy-transformparamstoconstref
//META: doc_section_digest=bdb28b3b073204cb31c496d7a9f8bbd681bf8780fa4c772f4520a59c02006f6b
//META: purpose=moveGlobalVarInitializationToEntryPoints (Metal fallthrough) fires the initializer of a `static` module-scope variable inside the kernel; the global itself appears as a field of the kernel-context struct.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// On Metal the fallthrough chain runs
// `moveGlobalVarInitializationToEntryPoints` so a `static int
// g_counter = 42;` global has its `= 42` assignment emitted
// inside the kernel body, against the global stored on the
// kernel-context struct.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

static int g_counter = 42;
RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = g_counter + int(tid.x);
}

// CHECK: int g_counter_{{[0-9]+}};
// CHECK: g_counter_{{[0-9]+}} = int(42)
