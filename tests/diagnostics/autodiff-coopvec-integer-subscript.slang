//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature

void test()
{
    let result = fwd_diff(CoopVec<int, 4>::__subscript::get)(0);
//CHECK:         ^^^^^^^^ member not found
//CHECK:         ^^^^^^^^ 'fwd_diff' is not a member of 'CoopVec<int, 4>.operator[].get'.
}
