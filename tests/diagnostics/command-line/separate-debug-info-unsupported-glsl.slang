// separate-debug-info-unsupported-glsl.slang

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target glsl -separate-debug-info -entry main -stage vertex
//CHECK: '-separate-debug-info' is not supported for target 'glsl'

void main()
{
}