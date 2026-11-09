// Regression coverage for field-sensitive differential-pair loads.
//
// A pair field access starts as `getPrimal(load(pairPointer))`. Pair lowering must address the
// field before loading it so that reading `.p` does not also read an uninitialized `.d` field.

//TEST:SIMPLE(filecheck=IR): -target spirv-asm -entry computeMain -stage compute -dump-ir-after finalizeAutoDiffPass

void updatePrimal(inout DifferentialPair<float> value)
{
    value = diffPair(value.p, 1.0f);
}

void updateDifferential(inout DifferentialPair<float> value)
{
    value = diffPair(2.0f, value.d);
}

RWStructuredBuffer<float> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    DifferentialPair<float> value = diffPair(2.0f, 0.0f);
    updatePrimal(value);
    updateDifferential(value);
    output[0] = value.p;
}

// IR: ### AFTER finalizeAutoDiffPass:
// IR: [nameHint("updatePrimal")]
// IR: func %updatePrimal
// IR: block
// IR-NOT: %DiffPair{{.*}} = load
// IR: get_field_addr({{.*}}, %primal)
// IR-NEXT: {{.*}}load(
// IR: [nameHint("updateDifferential")]
// IR: func %updateDifferential
// IR: block
// IR-NOT: %DiffPair{{.*}} = load
// IR: get_field_addr({{.*}}, %differential)
// IR-NEXT: {{.*}}load(
