// separate-debug-info-unsupported-glsl.slang

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-target glsl -separate-debug-info -entry main -stage vertex

// CHECK: warning 18: '-separate-debug-info' is not supported for target 'glsl'

void main()
{
}