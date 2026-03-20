// separate-debug-info-unsupported-target.slang

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -separate-debug-info -entry main -stage vertex
//CHECK: '-separate-debug-info' is not supported for target 'hlsl'

void main()
{
}