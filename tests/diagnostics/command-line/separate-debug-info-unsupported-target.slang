// separate-debug-info-unsupported-target.slang

// TODO(#14431): The diagnostic below is a warning, so `slangc` returns 0. It should be an
// error. When #14431 is fixed, change `exit-code=0` to the new exit code.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,exit-code=0):-target hlsl -separate-debug-info -entry main -stage vertex
//CHECK: '-separate-debug-info' is not supported for target 'hlsl'

void main()
{
}