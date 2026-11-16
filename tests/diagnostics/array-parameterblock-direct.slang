// An explicitly sized `ParameterBlock` array remains `SignatureChecked` and reaches semantic body
// checking. Exhaustive matching proves that body checking reports the unsupported element type
// exactly once.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
}

uniform ParameterBlock<float> qs[2];
//CHECK:                      ^^ arrays of non-addressable type not allowed
//CHECK:                      ^^ Arrays of non-addressable type 'ParameterBlock<float>' are not allowed
