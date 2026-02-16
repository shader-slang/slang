// separate-debug-info-unsupported-target.slang

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-target hlsl -separate-debug-info -entry main -stage vertex

// CHECK: warning 18: '-separate-debug-info' is not supported for target 'hlsl'

void main()
{
}