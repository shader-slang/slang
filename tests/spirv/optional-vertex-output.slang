//TEST:SIMPLE(filecheck=CHECK,NO_LAYOUT): -target spirv -warnings-disable 38052

// Test that we can use Optional<T> or bool types in varying input or outputs.

// CHECK-DAG: OpDecorate %i_inA_value Location 0
// CHECK-DAG: OpDecorate %i_inA_hasValue Location 1
// CHECK-DAG: OpDecorate %i_inputValues Location 2
// CHECK-DAG: OpDecorate %entryPointParam_vertMain_a_value Location 0
// CHECK-DAG: OpDecorate %entryPointParam_vertMain_a_hasValue Location 1
// CHECK-DAG: OpDecorate %entryPointParam_vertMain_outputValues Location 2

// NO_LAYOUT: ; Annotations
// NO_LAYOUT-NOT: OpMemberDecorate %{{.*}} Offset
// NO_LAYOUT-NOT: OpDecorate %{{.*}} ArrayStride
// NO_LAYOUT: ; Types, variables and constants

struct VIn {
    Optional<float> inA;
    bool inputValues[3];
}

struct VSOut {
    Optional<float> a;
    bool outputValues[3];
};

[shader("vertex")]
VSOut vertMain(VIn i)
{
    VSOut o;
    if (i.inA.hasValue)
        o.a = i.inA;
    else
        o.a = 0.0;
    o.outputValues = i.inputValues;
    return o;
}
