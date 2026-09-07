//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools
//META: doc_section_digest=f18a62d46309fae1d33b5fed992f19c2eff74d722d039dfeeeccc0dfdbf5b824
//META: purpose=The `SourceWriter` emits `#line N "file"` directives into Metal output by default so downstream errors map back to Slang source.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the Metal emitter inherits the shared C-like source writer's
// line-mapping behavior. Slang does not check the MSL it produces -- all
// grammar checking is delegated to the Apple tool -- so the line
// directives are what makes a downstream error message traceable. CHECK
// asserts a `#line <number>` directive is present; the file name is left
// unmatched because core-module-sourced items reference other files.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// CHECK: #line {{[0-9]+}}
