
//TEST:SIMPLE(filecheck=CHECK_2026): -target spirv -std 202b
//TEST:SIMPLE(filecheck=CHECK_2026): -target spirv -std 2026
//TEST:SIMPLE(filecheck=CHECK_2026): -target spirv -DVER_2026
//TEST:SIMPLE(filecheck=CHECK_2026): -target spirv -DVER_2026_2
//TEST:SIMPLE(filecheck=CHECK_2026): -target spirv -DVER_LATEST1
//TEST:SIMPLE(filecheck=CHECK_2026): -target spirv -DVER_LATEST2

//TEST:SIMPLE(filecheck=CHECK_2026): -target spirv -std 202c
//TEST:SIMPLE(filecheck=CHECK_2026): -target spirv -std next
//TEST:SIMPLE(filecheck=CHECK_2026): -target spirv -DVER_2027
//TEST:SIMPLE(filecheck=CHECK_2026): -target spirv -DVER_2027_2

//TEST:SIMPLE(filecheck=CHECK_LEGACY): -target spirv
//DIAGNOSTIC_TEST:SIMPLE(diag=DIAG1): -target spirv -DVER_ILLEGAL
//DIAGNOSTIC_TEST:SIMPLE(diag=DIAG2): -target spirv -DVER_ILLEGAL2
//DIAGNOSTIC_TEST:SIMPLE(diag=DIAG3): -target spirv -DVER_ILLEGAL3
//DIAGNOSTIC_TEST:SIMPLE(diag=DIAG4): -target spirv -DVER_ILLEGAL4
//DIAGNOSTIC_TEST:SIMPLE(diag=DIAG5): -target spirv -DLANG_ILLEGAL
//DIAGNOSTIC_TEST:SIMPLE(diag=DIAG6): -target spirv -DLANG_ILLEGAL_2

// Note: tests/compute/cast-zero-to-struct.slang has testing for Slang 2026 vs 2027 behavior

#ifdef VER_2026
#language slang 2026
#endif

#ifdef VER_2026_2
#language 2026
#endif

#ifdef VER_2027
#language slang 202c
#endif

#ifdef VER_2027_2
#language 202c
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
 ^^^^ unknown language version
 ^^^^ unknown language version 'illegal'
*/
#endif

#ifdef VER_ILLEGAL2
#lang 9987
/*DIAG2:
 ^^^^ unknown language version
 ^^^^ unknown language version '9987'
*/
#endif

#ifdef VER_ILLEGAL3
#lang "9987"
/*DIAG3:
 ^^^^ expected integer version
 ^^^^ expected integer for #version number
*/
#endif

#ifdef VER_ILLEGAL4
#lang
/*DIAG4:
 ^^^^ expected integer version
 ^^^^ expected integer for #version number
*/
#endif

#ifdef LANG_ILLEGAL
#lang illegal
/*DIAG5:
 ^^^^ unknown language
 ^^^^ unknown language 'illegal'
*/
#endif

#ifdef LANG_ILLEGAL_2
#language glsl 450
/*DIAG6:
 ^^^^^^^^ unknown language
 ^^^^^^^^ unknown language 'glsl'
*/
#endif

// CHECK_2026: OpEntryPoint

[numthreads(1,1,1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // CHECK_LEGACY: error[E30027]
    let x = (1,2,3)._1_2;

}
