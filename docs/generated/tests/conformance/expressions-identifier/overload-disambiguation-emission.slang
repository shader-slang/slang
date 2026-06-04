//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:36:27+00:00
//META: source_commit=273e29cb6cac600f70da7245a506bae6689e0e23
//META: doc_ref=docs/language-reference/expressions-identifier.md#overloading
//META: doc_section_digest=9484a135a7d65023e550053886ef224a8110c3717ecb4a6f046fb51a7be6e355
//META: purpose=Overload disambiguation is reflected in emitted HLSL as distinct mangled function names for each resolved overload.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Emission counterpart for overload-disambiguation-functional.slang.
// The compiler must emit distinct function definitions for the two overloads of `probe`,
// confirming that disambiguation produced two separate specializations.
//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute

int probe(int x)   { return 1; }
int probe(float x) { return 2; }

RWStructuredBuffer<int> outBuf;

[numthreads(1, 1, 1)]
void computeMain()
{
    int ri = probe(int(3));
    int rf = probe(float(3));
    outBuf[0] = ri + rf;
}

// The emitted HLSL has two distinct probe definitions and two distinct call sites.
// CHECK-DAG: int probe_{{[0-9]+}}(int
// CHECK-DAG: int probe_{{[0-9]+}}(float
