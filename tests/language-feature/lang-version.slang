
//TEST:SIMPLE(filecheck=CHECK_2026): -target spirv -std 2026
//TEST:SIMPLE(filecheck=CHECK_2026): -target spirv -DVER_2026
//TEST:SIMPLE(filecheck=CHECK_2026): -target spirv -DVER_2026_2
//TEST:SIMPLE(filecheck=CHECK_2026): -target spirv -DVER_LATEST1
//TEST:SIMPLE(filecheck=CHECK_2026): -target spirv -DVER_LATEST2
//TEST:SIMPLE(filecheck=CHECK_LEGACY): -target spirv
//DIAGNOSTIC_TEST:SIMPLE(diag=DIAG1): -target spirv -DVER_ILLEGAL
//DIAGNOSTIC_TEST:SIMPLE(diag=DIAG2): -target spirv -DVER_ILLEGAL2
//DIAGNOSTIC_TEST:SIMPLE(diag=DIAG3): -target spirv -DLANG_ILLEGAL

#ifdef VER_2026
#language slang 2026
#endif

#ifdef VER_2026_2
#language 2026
#endif

#ifdef VER_LATEST1
#lang slang latest
#endif

#ifdef VER_LATEST2
#lang latest
#endif

#ifdef VER_ILLEGAL
#lang slang illegal
/*DIAG1:
 ^^^^ unknown language
 ^^^^ unknown language 'illegal'
 ^^^^ unknown language version
 ^^^^ unknown language version '0'
*/
#endif

#ifdef VER_ILLEGAL2
#lang 9987
/*DIAG2:
 ^^^^ unknown language version
 ^^^^ unknown language version '9987'
*/
#endif

#ifdef LANG_ILLEGAL
#lang illegal
/*DIAG3:
 ^^^^ unknown language
 ^^^^ unknown language 'illegal'
 ^^^^ unknown language version
 ^^^^ unknown language version '0'
*/
#endif

// CHECK_2026: OpEntryPoint

[numthreads(1,1,1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // CHECK_LEGACY: ([[# @LINE+1]]): error 30027
    let x = (1,2,3)._1_2;

}
