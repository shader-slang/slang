// Test that invalid varying types are caught in `out`, `inout`, and `__ref`
// parameters, with correct direction labelling in the diagnostic text.
//
// `InOutModifier` inherits from `OutModifier` in the AST, so the direction
// detection logic must check `InOutModifier` first to avoid mislabeling
// `inout` parameters as "output". `RefModifier` is a separate modifier and
// should also label as "input/output".

//DIAGNOSTIC_TEST:SIMPLE(diag=OUT,non-exhaustive):-target spirv -entry outParam -stage fragment
//DIAGNOSTIC_TEST:SIMPLE(diag=INOUT,non-exhaustive):-target spirv -entry inoutParam -stage fragment
//DIAGNOSTIC_TEST:SIMPLE(diag=REF,non-exhaustive):-target spirv -entry refParam -stage fragment

[shader("fragment")]
void outParam(out DifferentialPair<float> dp : SV_Target0)
//OUT: used as output
{
}

[shader("fragment")]
void inoutParam(inout DifferentialPair<float> dp : SV_Target0)
//INOUT: used as input/output
{
}

[shader("fragment")]
void refParam(__ref DifferentialPair<float> dp : SV_Target0)
//REF: used as input/output
{
}
