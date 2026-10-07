//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature

void test(CoopVec<int, 4> value)
{
    let result = fwd_diff(CoopVec<int, 4>::__subscript::get)(value, 0);
//CHECK:         ^^^^^^^^ member not found
//CHECK:         ^^^^^^^^ 'fwd_diff' is not a member of 'CoopVec<int, 4>.operator[].get'.
}
