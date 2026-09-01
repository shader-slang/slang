//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#undoparametercopy--transformparamstoconstref
//META: doc_section_digest=79ddec9513950e753aa3cb10c03d98ab4885a8fd448be4549c7698082e2ef35f
//META: purpose=`undoParameterCopy` removes the front end's copy-in/copy-out wrapper for an `inout` struct parameter, so the callee takes a `thread*` pointer and the caller passes the address directly.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies Metal takes the CPP/CUDA/Metal arm of the parameter-handling
// switch. The front end models `inout` as copy-in/copy-out around a local
// temporary; undoing that gives the pass-by-pointer shape the downstream
// Apple compiler expects. The helper mutates its `inout` argument twice so
// a surviving copy would be observable, and CHECK asserts the emitted
// helper signature is `Acc_<N> thread*` and the call site passes `&`-of the
// caller's local rather than a fresh temporary.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

struct Acc { float total; int count; };

RWStructuredBuffer<float> outBuf;

void accumulate(inout Acc a, float v) { a.total += v; a.count += 1; }

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Acc a = { 0.0f, 0 };
    accumulate(a, float(tid.x));
    accumulate(a, float(tid.y));
    outBuf[tid.x] = a.total + float(a.count);
}

// CHECK: void accumulate_{{[0-9]+}}(Acc_{{[0-9]+}} thread*
// CHECK: accumulate_{{[0-9]+}}(&a_{{[0-9]+}}
