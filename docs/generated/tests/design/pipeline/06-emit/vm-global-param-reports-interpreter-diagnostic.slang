//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/06-emit.md#vm
//META: doc_section_digest=d88e5991fabadc3c3b8e0b33895fac31d24dbf358696153641375ca3aee01159
//META: purpose=The VM bytecode emitter detects any function that references a global shader parameter and reports the interpreter-unsupported diagnostic once per parameter instead of handing back malformed bytecode.
//META: intent=negative
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the bytecode emitter fails cleanly rather than emitting
// bytecode the interpreter cannot run. The interpreter is a CPU VM with no
// notion of global shader parameters, so a module-scope `RWStructuredBuffer`
// is unrepresentable. The buffer is read through a helper called twice, which
// exercises the "once per parameter" part of the claim: CHECK pins the stable
// code E52013 and then CHECK-NOT forbids a second occurrence. The non-zero
// result code confirms the emitter returned a failure rather than continuing.

//TEST:INTERPRET(filecheck=CHECK):

RWStructuredBuffer<float> inputs;

float load(uint index)
{
    return inputs[index];
}

void main()
{
    //CHECK: result code = 1
    //CHECK: error[E52013]
    //CHECK-NOT: error[E52013]
    printf("%f\n", load(0) + load(1));
}
