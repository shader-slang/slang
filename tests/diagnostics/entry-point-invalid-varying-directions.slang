// Test that invalid varying types are caught in `out` and `inout` parameters, with correct
// direction labelling in the diagnostic text.
// An `inout` parameter is both a stage input and a stage output, so it must
// not be labelled as only "output".

//DIAGNOSTIC_TEST:SIMPLE(diag=OUT,non-exhaustive):-target spirv -entry outParam -stage fragment
//DIAGNOSTIC_TEST:SIMPLE(diag=INOUT,non-exhaustive):-target spirv -entry inoutParam -stage fragment
//DIAGNOSTIC_TEST:SIMPLE(diag=IN_OUT,non-exhaustive):-target spirv -entry inAndOutParam -stage fragment

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

// The two-keyword spelling `in out` declares the same passing mode as `inout`.
[shader("fragment")]
void inAndOutParam(in out DifferentialPair<float> dp : SV_Target0)
//IN_OUT: used as input/output
{
}
