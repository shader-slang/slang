// separate-debug-info-stdout.slang

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK1):-target spirv -g2 -emit-spirv-directly -separate-debug-info -entry main -stage compute
//CHECK1: 00109

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK2):-target spirv -g2 -emit-spirv-directly -separate-debug-info -entry main -stage compute -o -
//CHECK2: 00109

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK3):-target spirv -g2 -emit-spirv-directly -separate-debug-info -whole-program
//CHECK3: 00109

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK4):-target spirv -g2 -emit-spirv-directly -separate-debug-info -whole-program -o -
//CHECK4: 00109

[numthreads(1, 1, 1)]
void main()
{
}
