// separate-debug-info-output-without-separate-debug-info.slang

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage compute -separate-debug-info-output unused.dbg.spv

// CHECK: error[E00110]: `-separate-debug-info-output` requires `-separate-debug-info`

[numthreads(1, 1, 1)]
void main()
{
}
