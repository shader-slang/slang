//DIAGNOSTIC_TEST:SIMPLE(diag=diag):
// Test that #pragma warning (push) emits a warning when the stack is not empty at the end

#pragma warning (push)
//diag:          ^^^^ unmatched push
//diag:          ^^^^ detected #pragma warning(push) with no corresponding #pragma warning(pop)
